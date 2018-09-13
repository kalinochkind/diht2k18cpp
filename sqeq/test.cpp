#include "solve.h"
#include "test.h"

TEST("linear, infinite") {
  ASSERT(solve_linear_equation(0., 0.).is_any_root());
}

TEST("linear, none") {
  ASSERT(solve_linear_equation(0., 1.).roots().empty());
}

TEST("linear, zero root") {
  auto solution = solve_linear_equation(-1., 0.);
  ASSERT(solution.roots().size() == 1);
  ASSERT_FLOAT_EQUAL(solution.roots()[0], 0);
}

TEST("linear, one root") {
  auto solution = solve_linear_equation(6., -2.);
  ASSERT(solution.roots().size() == 1);
  ASSERT_FLOAT_EQUAL(solution.roots()[0], 1. / 3);
}

TEST("quadratic, a=0") {
  auto solution = solve_quadratic_equation(0., 6., 2.);
  ASSERT(solution.roots().size() == 1);
  ASSERT_FLOAT_EQUAL(solution.roots()[0], -1. / 3);
}

TEST("quadratic, no roots") {
  ASSERT(solve_quadratic_equation(1, 1, 1).roots().empty());
}

TEST("quadratic, one root") {
  auto solution = solve_quadratic_equation(1., 2., 1.);
  ASSERT(solution.roots().size() == 1);
  ASSERT_FLOAT_EQUAL(solution.roots()[0], -1);
}

TEST("quadratic, two roots") {
  auto solution = solve_quadratic_equation(2., -10., 12.);
  ASSERT(solution.roots().size() == 2);
  ASSERT_FLOAT_EQUAL(solution.roots()[0], 2);
  ASSERT_FLOAT_EQUAL(solution.roots()[1], 3);
}

TEST("quadratic, two roots, a<0") {
  auto solution = solve_quadratic_equation(-1., 5., -6.);
  ASSERT(solution.roots().size() == 2);
  ASSERT_FLOAT_EQUAL(solution.roots()[0], 2);
  ASSERT_FLOAT_EQUAL(solution.roots()[1], 3);
}

int main() {
  return run_tests();
}