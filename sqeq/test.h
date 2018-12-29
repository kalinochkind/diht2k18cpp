#pragma once

#include <functional>
#include <vector>
#include <iostream>


using testcases_t = std::vector<std::pair<const char*, std::function<void()>>>;

testcases_t& _get_testcases() {
  static testcases_t t;
  return t;
}

#define _CAT_(A, B) A ## B
#define _CAT(A, B) _CAT_(A, B)

struct _TestCase {
  _TestCase(std::function<void()> f, const char* s) {
    _get_testcases().emplace_back(s, f);
  }
};

#define TEST(s) void _CAT(_test, __LINE__) (); static _TestCase _CAT(_tc, __LINE__) {_CAT(_test, __LINE__), s}; void _CAT(_test, __LINE__) ()

class AssertionFailed {
 public:
  AssertionFailed(const std::string &message)
      : message_(message) {
  };

  std::string what() const {
    return "Assertion failed: " + message_;
  }

 private:
  std::string message_;
};


int run_tests() {
  bool failed = false;
  for (const auto& pair : _get_testcases()) {
    try {
      pair.second();
    } catch (const AssertionFailed& e) {
      std::cerr << "Test " << pair.first << " failed\n" << e.what() << "\n\n";
      failed = true;
    } catch (const std::exception& e) {
      std::cerr << "Test " << pair.first << " crashed\n" << e.what() << "\n\n";
      failed = true;
    }
  }
  if (!failed) {
    std::cerr << "All tests passed\n";
  }
  return failed;
}

#define ASSERT(expr) if(!(expr)) throw AssertionFailed(std::string(#expr) + " is false")
#define ASSERT_FLOAT_EQUAL(a, b) if(fabs((a) - (b)) > 0.00000001) throw AssertionFailed(std::string(#a) + " != " + #b)