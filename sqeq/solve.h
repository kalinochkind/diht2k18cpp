/** @file solve.h */
#pragma once

#include <vector>
#include <cmath>
#include <algorithm>
#include <cassert>

constexpr double EPS = 0.00000001;


/**
 * A set of roots of the equation.
 * Either contains a finite number of roots or indicates that any number is a solution.
 */
template <class T>
class EquationRootSet {
 public:
  EquationRootSet(const std::initializer_list<T>& roots)
      : roots_(roots) {
  }

  /**
   * Constructs an EquationRootSet instance indicating that every number is a root of an equation.
   */
  static EquationRootSet all_roots() {
    EquationRootSet set{};
    set.any_root_ = true;
    return set;
  }

  /**
   * @return A vector of roots (empty if any number is a root)
   */
  std::vector<T> roots() const {
    return roots_;
  }

  /**
   * @return true, if any number is a root, false otherwise
   */
  bool is_any_root() const {
    return any_root_;
  }

 private:

  std::vector<T> roots_;
  bool any_root_;
};

/**
 * Solves linear equation ax + b = 0
 * @tparam T type of a, b and return value
 * @return EquationRootSet instance representing roots of the equation
 */
template <class T>
const EquationRootSet<T> solve_linear_equation(const T a, const T b) {
  assert(std::isfinite(a));
  assert(std::isfinite(b));
  if (a == 0) {
    if (b == 0) {
      return EquationRootSet<T>::all_roots();
    } else {
      return {};
    }
  }
  return {-b / a};
}

/**
 * Solves quadratic equation ax^2 + bx + c = 0
 * @tparam T type of a, b, c and return value
 * @return EquationRootSet instance representing roots of the equation
 */
template <class T>
const EquationRootSet<T> solve_quadratic_equation(const T a, const T b, const T c) {
  assert(std::isfinite(a));
  assert(std::isfinite(b));
  assert(std::isfinite(c));
  if (a == 0) {
    return solve_linear_equation(b, c);
  }
  T discriminant = b * b - 4 * a * c;
  if (discriminant < -EPS) {
    return {};
  }
  T middle = -b / (2 * a);
  if (discriminant < EPS) {
    return {middle};
  }
  T diff = sqrt(discriminant) / (2 * a);
  if (a < 0) {
    diff = -diff;
  }
  assert(diff > 0);
  return {middle - diff, middle + diff};
}
