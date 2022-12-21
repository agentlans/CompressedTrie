#include <iostream>
#include <cassert>

#include "CompressedTrie.hpp"

int main() {
    Node<int> foo;
    foo.get_child("bar");

    CompressedTrie<int> bar;
    bar.insert("hello world", 123);
    std::cout << bar.find("hello world");
    assert(!bar.remove("hello"));
    assert(bar.remove("hello world"));

    auto bar2 = bar;
    assert(!bar2.contains("hello world"));
}