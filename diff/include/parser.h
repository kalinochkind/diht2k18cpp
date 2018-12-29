#pragma once

#include <stdexcept>
#include <string>
#include "expression.h"


class ParseError : public std::runtime_error {
  using std::runtime_error::runtime_error;
};


expression_ptr_t parse_expression(const std::string& s);
