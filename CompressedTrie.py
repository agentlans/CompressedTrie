class Node:
  """A node in a radix tree."""
  def __init__(self):
    self.parent = None
    self.edges = []
    self.endpoint = False # Whether node is at the end of a string
  def get_child(self, edge):
    for e, v in self.edges:
      if e == edge:
        return v
    return None
  def add_edge(self, edge, child):
    child.parent = self
    self.edges.append((edge, child))
  def remove_edge(self, edge):
    self.edges = [(e,v) for e,v in self.edges if e != edge]
  def remove_node(self, child):
    self.edges = [(e,v) for e,v in self.edges if v != child]
  def is_root(self):
    return (self.parent is None)
  def as_str(self): # for debugging purposes
    s = ""
    for e,v in self.edges:
      s += "(\"" + str(e) + "\" " + v.as_str() + ")"
    return s

def longest_common_prefix(s1, s2, start, end):
  """Returns the largest i such that s1[0:i) == s2[start:start+i)
  and start + i <= end."""
  i = 0
  while start + i < end and i < len(s1) and s1[i] == s2[start+i]:
    i += 1
  return i
#  longest_common_prefix("012345", "asdfx30123xbsw", 6, 14)

def lookup(root, string, start, end):
  """Returns (node, edge, i, j) such that 
  node is a descendant of root,
  edge is empty or comes out of node,
  string[i,j) == edge[0,j-i),
  where start <= i <= j <= end"""
  if start == end:
    # Empty string
    return root, "", start, end
  for edge, child in root.edges:
    d = longest_common_prefix(edge, string, start, end)
    if d == 0:
      continue # String nothing in common with this edge
    if d < len(edge):  # Partial match
      return root, edge, start, start + d
    if d == len(edge):
      # Complete match. Look for more at the child node.
      return lookup(child, string, start + d, end)
  # string[start,end) doesn't match any edge at all
  return root, "", start, start

class CompressedTrie:
  def __init__(self):
    self.root = Node()
  def _lookup(self, string):
    return lookup(self.root, string, 0, len(string))
  def find(self, string):
    "Returns node that corresponds to the given string, or None if not found."
    node, _, i, j = self._lookup(string)
    if i == j == len(string) and node.endpoint:
      return node
    else:
      return None
  def insert(self, string):
    """Inserts string into the tree and returns the node.
    If tree already has the string, then returns the existing node."""
    node, edge, i, j = self._lookup(string)
    n = len(string)
    if i == j == n:
      # Found the node
      node.endpoint = True
      return node
    elif i == j and j < n:
      # string[i:) isn't in the tree
      new_node = Node()
      new_node.endpoint = True
      node.add_edge(string[i:], new_node)
      return new_node
    elif i < j:
      # Partial match at the node.
      # string[i:i+d) is part of edge but string[i+d:) isn't.
      d = j - i
      # Split the node
      child = node.get_child(edge)
      node.remove_edge(edge)
      # Create a middle node
      middle_node = Node()
      node.add_edge(edge[0:d], middle_node)
      middle_node.add_edge(edge[d:], child)
      # If the string is a prefix of the edge
      if j == len(string):
        middle_node.endpoint = True
        return middle_node
      # Otherwise, add the unmatched part of string to the middle_node
      new_node = Node()
      middle_node.add_edge(string[j:], new_node)
      new_node.endpoint = True
      return new_node
  def remove(self, string):
    """Removes the node corresponding to the given string from the tree.
    Returns True if removed or False if string isn't in the tree."""
    node = self.find(string)
    if node is None:
      # String not in the tree.
      return False
    # Node isn't an endpoint any more.
    node.endpoint = False
    while not node.is_root() and not node.endpoint and node.edges == []:
      node.parent.remove_node(node)
      node = node.parent
    return True
