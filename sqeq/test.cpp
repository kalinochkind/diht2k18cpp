#include "solve.h"
#include "test.h"

TEST("linear, infinite") {
  std::vector<double> roots;
  ASSERT(solve_linear_equation(0., 0., std::back_inserter(roots)) == EQ_INFINITE_ROOTS);
}

TEST("linear, none") {
  std::vector<double> roots;
  ASSERT(solve_linear_equation(0., 1., std::back_inserter(roots)) == 0);
}

TEST("linear, zero root") {
  std::vector<double> roots;
  ASSERT(solve_linear_equation(-1., 0., std::back_inserter(roots)) == 1);
  ASSERT_FLOAT_EQUAL(roots.at(0), 0);
}

TEST("linear, one root") {
  std::vector<double> roots;
  ASSERT(solve_linear_equation(6., -2., std::back_inserter(roots)) == 1);
  ASSERT_FLOAT_EQUAL(roots.at(0), 1. / 3);
}

TEST("quadratic, a=0") {
  std::vector<double> roots;
  ASSERT(solve_quadratic_equation(0., 6., 2., std::back_inserter(roots)) == 1);
  ASSERT_FLOAT_EQUAL(roots.at(0), -1. / 3);
}

TEST("quadratic, no roots") {
  std::vector<double> roots;
  ASSERT(solve_quadratic_equation(1, 1, 1, std::back_inserter(roots)) == 0);
}

TEST("quadratic, one root") {
  std::vector<double> roots;
  ASSERT(solve_quadratic_equation(1, 2, 1, std::back_inserter(roots)) == 1);
  ASSERT_FLOAT_EQUAL(roots.at(0), -1);
}

TEST("quadratic, two roots") {
  std::vector<double> roots;
  ASSERT(solve_quadratic_equation(2., -10., 12., std::back_inserter(roots)) == 2);
  ASSERT_FLOAT_EQUAL(roots.at(0), 2);
  ASSERT_FLOAT_EQUAL(roots.at(1), 3);
}

TEST("quadratic, two roots, a<0") {
  std::vector<double> roots;
  ASSERT(solve_quadratic_equation(-1., 5., -6., std::back_inserter(roots)) == 2);
  ASSERT_FLOAT_EQUAL(roots.at(0), 2);
  ASSERT_FLOAT_EQUAL(roots.at(1), 3);
}

int main() {
  return run_tests();
}