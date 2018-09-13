#include <iostream>
#include "solve.h"

int main() {
  std::cout << "Enter equation coefficients: ";
  double a, b, c;
  std::cin >> a >> b >> c;
  std::vector<double> roots;
  int solution = solve_quadratic_equation(a, b, c, std::back_inserter(roots));
  switch (solution) {
    case EQ_INFINITE_ROOTS:
      std::cout << "Any number is a root";
      break;
    case 0:
      std::cout << "No roots";
      break;
    case 1:
      assert(roots.size() >= 1);
      std::cout << "One root: " << roots[0];
      break;
    case 2:
      assert(roots.size() >= 2);
      std::cout << "Two roots: " << roots[0] << ", " << roots[1];
      break;
    default:
      std::cout << "Sorry, I failed" << std::endl;
      return 1;
  }
  std::cout << std::endl;
  return 0;
}
