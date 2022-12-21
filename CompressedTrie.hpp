#ifndef _COMPRESSEDTRIE
#define _COMPRESSEDTRIE

#include <memory>
#include <map>
#include <string>
#include <algorithm>

typedef std::string Edge;

template <class Value>
class Node : public std::enable_shared_from_this<Node<Value>> {
public:
    typedef std::shared_ptr<Node<Value>> SPtr;
    typedef std::weak_ptr<Node<Value>> WPtr;

    WPtr get_child(const Edge& edge);
    void add_edge(const Edge& edge, SPtr child);
    // Removes the child connected to this Node by the edge
    void remove(const Edge& edge);
    void remove(const SPtr& child);
    // Whether this node is root node
    bool is_root() const {return parent.expired();}
    // Value recorded in the Node
    Value& value() {return x;}
    const Value& value() const {return x;}

    bool is_endpoint() const {return endpoint;}
    void set_endpoint(bool status) {endpoint = status;}

    bool empty() const {return edges.empty();}

    const auto& get_edges() const {return edges;}
    WPtr get_parent() {return parent;}
private:
    WPtr parent;
    std::map<Edge, SPtr> edges;
    bool endpoint = false;
    Value x;
};

template <class Value>
using WPtr = std::weak_ptr<Node<Value>>;

template <class Value>
using SPtr = std::shared_ptr<Node<Value>>;

template <class T>
bool is_null(WPtr<T> p) {
    return p.expired();
}

template <class T>
WPtr<T> Node<T>::get_child(const Edge& edge) {
    auto it = edges.find(edge);
    if (it != edges.end()) {
        return it->second;
    } else {
        return WPtr();
    }
}

template <class T>
void Node<T>::add_edge(const Edge& edge, SPtr child) {
    child->parent = this->weak_from_this();
    edges[edge] = child;
}

template <class T>
void Node<T>::remove(const Edge& edge) {
    edges.erase(edge);
}

template <class T>
void Node<T>::remove(const SPtr& child) {
    // Find this particular child
    auto is_target = [&](const auto& kv) {
        return kv.second == child;
    };
    auto it = std::find_if(edges.begin(), edges.end(), is_target);
    edges.erase(it);
}

int longest_common_prefix(
    const std::string& s1, const std::string& s2, 
    int start, int end) {
    // Returns the largest i such that 
    // s1[0:i) == s2[start:start+i) and start + i <= end.
    int i = 0;
    while (start + i < end && i < s1.size() && s1[i] == s2[start+i]) {
        i++;
    }
    return i;
}

template <class T>
struct LookupResult {
    WPtr<T> node;
    std::string edge;
    int i, j;
};

template <class T>
LookupResult<T> lookup(WPtr<T> root, const std::string& string, int start, int end) {
    /*
  Returns (node, edge, i, j) such that 
  node is a descendant of root,
  edge is empty or comes out of node,
  string[i,j) == edge[0,j-i),
  where start <= i <= j <= end
    */
    if (start == end) {
        // Empty string
        return {root, "", start, end};
    }
    for (const auto& kv : root.lock()->get_edges()) {
        // kv is edge, child
        Edge edge;
        WPtr<T> child;
        std::tie(edge, child) = kv;
        int d = longest_common_prefix(edge, string, start, end);
        int n = edge.size();
        if (d < n) {
            // Partial match
            return {root, edge, start, start + d};
        }
        if (d == n) {
            // Complete match
            return lookup(child, string, start + d, end);
        }
    }
    // Doesn't match any edge
    return {root, "", start, start};
}

// Tree storing the prefixes of nodes in compressed form
template <class T>
class CompressedTrie {
public:
    CompressedTrie() : root(std::make_shared<Node<T>>()) {}
    T& find(const std::string& string);
    void insert(const std::string& string, const T& value);
    bool remove(const std::string& string);
    bool contains(const std::string& string) const;
private:
    SPtr<T> root;
    auto _lookup(const std::string& string) const {
        WPtr<T> rootp = root;
        return lookup(rootp, string, 0, string.size());
    }
    WPtr<T> find_node(const std::string& string) const;
};

// Searches the tree for the given string. Returns pointer to the node.
template <class T>
WPtr<T> CompressedTrie<T>::find_node(const std::string& string) const {
    auto res = _lookup(string);
    if (res.i == res.j && res.j == string.size() && 
        res.node.lock()->is_endpoint()) {
        return res.node;
    } else {
        // If not found, return null
        return std::make_shared<Node<T>>();
    }
}

template <class T>
bool CompressedTrie<T>::contains(const std::string& string) const {
    return !is_null(find_node(string));
}

template <class T>
T& CompressedTrie<T>::find(const std::string& string) {
    auto res = find_node(string);
    if (!is_null(res)) {
        return res.lock()->value();
    } else {
        throw "Key not found.";
    }
}

std::string slice(const std::string& s, int start, int end) {
    auto end_it = s.begin() + end;
    if (end < 0) {
        end_it = s.end();
    }
    return std::string(s.begin() + start, end_it);
}

template <class T>
void CompressedTrie<T>::insert(const std::string& string, const T& value) {
    LookupResult<T> res = _lookup(string);
    int n = string.size();
    if (res.i == res.j == n) {
        // Found the node
        res.node.lock()->set_endpoint(true);
        res.node.lock()->value() = value;
        return;
    } else if (res.i == res.j && res.j < n) {
        // string[i:) isn't in the tree
        auto new_node = std::make_shared<Node<T>>();
        new_node->set_endpoint(true);
        new_node->value() = value;
        // Make a new edge for the extra part
        res.node.lock()->add_edge(slice(string, res.i, -1), new_node);
    } else if (res.i < res.j) {
        int d = res.j - res.i;
        // Split the node
        auto child = res.node.lock()->get_child(res.edge);
        res.node.lock()->remove(res.edge);
        // Create a middle node
        auto middle_node = std::make_shared<Node<T>>();
        res.node.lock()->add_edge(slice(res.edge, 0, d), middle_node);
        middle_node->add_edge(slice(res.edge, d, -1), child.lock());
        // If the string is a prefix of the edge
        if (res.j == string.size()) {
            middle_node->set_endpoint(true);
            middle_node->value() = value;
            return;
        }
        // Otherwise, add the unmatched part of the string to the middle_node
        auto new_node = std::make_shared<Node<T>>();
        middle_node->add_edge(slice(string, res.j, -1), new_node);
        new_node->set_endpoint(true);
        new_node->value() = value;
    }
}

template <class T>
bool CompressedTrie<T>::remove(const std::string& string) {
    auto node = find_node(string).lock();
    if (!node) {
        // String not in tree
        return false;
    }
    // Node isn't an endpoint any more.
    node->set_endpoint(false);
    while (!node->is_root() && !node->is_endpoint() && node->empty()) {
        node->get_parent().lock()->remove(node);
        node = node->get_parent().lock();
    }
    return true;
}

#endif