#include <iostream>
#include "solve.h"

int main() {
  std::cout << "Enter equation coefficients: ";
  double a, b, c;
  std::cin >> a >> b >> c;
  auto solution = solve_quadratic_equation(a, b, c);
  if (solution.is_any_root()) {
    std::cout << "Any number suits";
  } else {
    auto roots = solution.roots();
    if (roots.empty()) {
      std::cout << "No roots";
    } else {
      std::cout << "Roots: ";
      for (double root : roots) {
        std::cout << root << ' ';
      }
    }
  }
  std::cout << std::endl;
  return 0;
}