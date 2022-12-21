from CompressedTrie import CompressedTrie
import random

def random_string(letters, length):
  """Returns a random string of random length from the alphabet of letters."""
  k = random.randint(0, length)
  return "".join(random.choices(letters, k=k))

class TestClass:
  def __init__(self):
    self.rt = CompressedTrie()
    self.control = set()
  def insert(self, string):
    "Inserts string into the tree or set"
    self.rt.insert(string)
    self.control.add(string)
  def remove(self, string):
    "Removes the string if found."
    self.rt.remove(string)
    self.control.discard(string)
  def find(self, string):
    "Tries to find string and checks against control."
    found = (self.rt.find(string) is not None)
    #print((found, (string in self.control)))
    assert found == (string in self.control)
  def random_op(self, string):
    "Do a random operation with the string"
    op = random.randint(1, 3)
    #print((op, string))
    if op == 1:
      self.insert(string)
    elif op == 2:
      self.remove(string)
    elif op == 3:
      self.find(string)

# Run the randomized tests
for trial in range(100):
  tc = TestClass()
  for i in range(10000):
    s = random_string(['a', 'b', 'c'], 5)
    tc.random_op(s)
