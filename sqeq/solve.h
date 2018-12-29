/** @file solve.h */
#pragma once

#include <vector>
#include <cmath>
#include <algorithm>
#include <cassert>


/**
 * If the difference between two doubles is less than this constant, they are considered equal.
 */
constexpr double EPS = 0.00000001;
/**
 * The constant indicating that any number is a root.
 */
constexpr int EQ_INFINITE_ROOTS = -1;


/**
 * Checks whether a number is close to zero.
 * @param number
 * @return true if the number is close to zero, false otherwise
 */
constexpr bool is_zero(const double number) {
    return number < EPS && number > -EPS;
}


/**
 * Solves linear equation ax + b = 0
 * @param a first coefficient
 * @param b second coefficient
 * @param roots iterator where the roots will be stored
 * @return number of roots or EQ_INFINITE_ROOTS
 */
template <class OutputIterator>
int solve_linear_equation(const double a, const double b, OutputIterator roots) {
  assert(std::isfinite(a));
  assert(std::isfinite(b));
  if (is_zero(a)) {
    if (is_zero(b)) {
      return EQ_INFINITE_ROOTS;
    } else {
      return 0;
    }
  }
  *(roots++) = -b / a;
  return 1;
}

/**
 * Solves quadratic equation ax^2 + bx + c = 0
 * @param a first coefficient
 * @param b second coefficient
 * @param c third coefficient
 * @param roots iterator where the roots will be stored
 * @return number of roots or EQ_INFINITE_ROOTS
 */
template <class OutputIterator>
int solve_quadratic_equation(const double a, const double b, const double c, OutputIterator roots) {
  assert(std::isfinite(a));
  assert(std::isfinite(b));
  assert(std::isfinite(c));
  if (is_zero(a)) {
    return solve_linear_equation(b, c, roots);
  }
  double discriminant = b * b - 4 * a * c;
  if (discriminant < -EPS) {
    return 0;
  }
  double middle = -b / (2 * a);
  if (discriminant < EPS) {
    *(roots++) = middle;
    return 1;
  }
  double diff = sqrt(discriminant) / (2 * a);
  if (a < 0) {
    diff = -diff;
  }
  assert(diff > 0);
  *(roots++) = middle - diff;
  *(roots++) = middle + diff;
  return 2;
}
