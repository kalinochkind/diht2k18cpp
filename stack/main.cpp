#include <iostream>
#include <cassert>
#include "stack.h"

int main() {
  std::string s = "abcdefg";
  std::cout << s[3] << ' ' << (int)(s[3]) << '\n';
  return 0;
}