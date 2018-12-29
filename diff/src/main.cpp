#include <iostream>
#include "expression.h"
#include "parser.h"


int main(int argc, char** argv) {
  std::cout << "Your expression: ";
  std::string s;
  std::getline(std::cin, s);
  expression_ptr_t exp = parse_expression(s)->derivative('x')->optimize();
  if (argc >= 2 && std::string(argv[1]) == "-t") {
    std::cout << "TeX derivative: " << exp->to_string(true) << std::endl;
  } else {
    std::cout << "Derivative: " << exp->to_string() << std::endl;
  }
  return 0;
}
