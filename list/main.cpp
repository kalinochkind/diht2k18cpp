#include <iostream>
#include <list>
#include "list.h"


int main() {
  LinkedList<int> t;
  t.push_back(1);
  t.push_front(0);
  t.push_back(2);
  t.insert_after(++++t.begin(), 5);
  t.insert_before(t.begin(), 6);
  t.insert_before(t.end(), 7);
  t.erase(++t.begin());
  t.erase(t.begin());
  t[3]++;
  *t.find(5) = 6;
  std::cout << t.dump() << '\n';
  return 0;
}