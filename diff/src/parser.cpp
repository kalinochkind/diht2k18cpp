#include <cassert>
#include <vector>
#include "parser.h"


enum class token_type_t {
  INT, REAL, VAR, FUNC, LEFTPAR, RIGHTPAR, UNARY_OP, BINARY_OP
};

using token_t = std::pair<token_type_t, std::string>;

static token_t number_token(const std::string& s) {
  if (s.find('.') == std::string::npos) {
    return {token_type_t::INT, s};
  }
  return {token_type_t::REAL, s};
}

static token_t func_token(const std::string& s) {
  if (s.length() == 1) {
    return {token_type_t::VAR, s};
  }
  return {token_type_t::FUNC, s};
}


static std::vector<token_t> tokenize(const std::string& s) {
  std::vector<token_t> res;
  std::string func, num;
  bool can_omit_mul = false;
  for (char c : s + ' ') {
    if ('a' <= c && c <= 'z') {
      if (!num.empty()) {
        res.push_back(number_token(num));
        can_omit_mul = true;
        num.clear();
      }
      if (can_omit_mul) {
        res.emplace_back(token_type_t::BINARY_OP, "*");
        can_omit_mul = false;
      }
      func.push_back(c);
    } else if (('0' <= c && c <= '9') || c == '.') {
      if (!func.empty()) {
        res.push_back(func_token(func));
        can_omit_mul = false;
        func.clear();
      }
      num.push_back(c);
    } else {
      if (!num.empty()) {
        res.push_back(number_token(num));
        num.clear();
      }
      if (!func.empty()) {
        res.push_back(func_token(func));
        func.clear();
      }
      if (c == '(') {
        if (can_omit_mul) {
          res.emplace_back(token_type_t::BINARY_OP, "*");
          can_omit_mul = false;
        }
        res.emplace_back(token_type_t::LEFTPAR, "");
        continue;
      }
      can_omit_mul = false;
      if (c == '-' && (res.empty() || res.back().first == token_type_t::LEFTPAR)) {
        res.emplace_back(token_type_t::UNARY_OP, "-");
      } else if (c == ')') {
        res.emplace_back(token_type_t::RIGHTPAR, "");
        can_omit_mul = true;
      } else if (c == '+' || c == '*' || c == '/' || c == '^' || c == '-') {
        res.emplace_back(token_type_t::BINARY_OP, std::string(1, c));
      }
    }
  }
  return res;
}

static const unsigned short precedence[128] =
    {['+']=1, ['-']=1, ['*']=2, ['/']=3, ['^']=4};
static const bool rightassoc[128] =
    {['^']=true};

static std::vector<token_t> to_rpn(const std::vector<token_t>& tokens) {
  std::vector<token_t> output;
  std::vector<token_t> operators;
  for (const token_t& token : tokens) {
    switch (token.first) {
      case token_type_t::INT:
      case token_type_t::REAL:
      case token_type_t::VAR:
        output.push_back(token);
        break;
      case token_type_t::UNARY_OP:
      case token_type_t::BINARY_OP:
        while (!operators.empty() && operators.back().first != token_type_t::LEFTPAR &&
               (operators.back().first == token_type_t::FUNC ||
                (operators.back().first == token_type_t::BINARY_OP &&
                 precedence[operators.back().second[0]] > precedence[token.second[0]]) ||
                operators.back().first == token_type_t::UNARY_OP ||
                (precedence[operators.back().second[0]] == precedence[token.second[0]] &&
                 !rightassoc[operators.back().second[0]]))) {
          output.push_back(operators.back());
          operators.pop_back();
        }
      case token_type_t::FUNC:
      case token_type_t::LEFTPAR:
        operators.push_back(token);
        break;
      case token_type_t::RIGHTPAR:
        while (!operators.empty() && operators.back().first != token_type_t::LEFTPAR) {
          output.push_back(operators.back());
          operators.pop_back();
        }
        if (operators.empty()) {
          throw ParseError("Mismatched parentheses");
        }
        operators.pop_back();
    }
  }
  while (!operators.empty()) {
    output.push_back(operators.back());
    operators.pop_back();
  }
  return output;
}


template <class Node>
static bool try_append_element(expression_ptr_t& a, expression_ptr_t& b) {
  auto as_node = dynamic_cast<Node*>(a.get());
  if (!as_node) {
    return false;
  }
  as_node->elements.push_back(std::move(b));
  return true;
}

expression_ptr_t parse_expression(const std::string& s) {
  auto rpn = to_rpn(tokenize(s));
  std::vector<expression_ptr_t> stack;
  expression_ptr_t temp;
  for (const token_t& token : rpn) {
    size_t pre_last = stack.size() - 2;
    switch (token.first) {
      case token_type_t::LEFTPAR:
      case token_type_t::RIGHTPAR:
        assert(false);
      case token_type_t::REAL:
        stack.push_back(std::make_unique<Constant>(Rational(token.second)));
        break;
      case token_type_t::INT:
        stack.push_back(std::make_unique<Constant>(Rational(token.second)));
        break;
      case token_type_t::VAR:
        stack.push_back(std::make_unique<Variable>(token.second[0]));
        break;
      case token_type_t::FUNC:
        if (stack.empty()) {
          throw ParseError("Invalid expression");
        }
        stack.back() = std::make_unique<Function>(token.second, std::move(stack.back()));
        break;
      case token_type_t::UNARY_OP:
        assert(token.second[0] == '-');
        if (stack.empty()) {
          throw ParseError("Invalid expression");
        }
        stack.back() = std::make_unique<Negation>(std::move(stack.back()));
        break;
      case token_type_t::BINARY_OP:
        if (stack.size() < 2) {
          throw ParseError("Invalid expression");
        }
        switch (token.second[0]) {
          case '^':
            stack[pre_last] = std::make_unique<Power>(std::move(stack[pre_last]), std::move(stack.back()));
            stack.pop_back();
            break;
          case '/':
            stack[pre_last] = std::make_unique<Division>(std::move(stack[pre_last]), std::move(stack.back()));
            stack.pop_back();
            break;
          case '*':
            if (!try_append_element<Multiplication>(stack[pre_last], stack.back())) {
              stack[pre_last] = std::make_unique<Multiplication>(std::move(stack[pre_last]), std::move(stack.back()));
            }
            stack.pop_back();
            break;
          case '-':
            stack.back() = std::make_unique<Negation>(std::move(stack.back()));
          case '+':
            if (!try_append_element<Addition>(stack[pre_last], stack.back())) {
              stack[pre_last] = std::make_unique<Addition>(std::move(stack[pre_last]), std::move(stack.back()));
            }
            stack.pop_back();
            break;
          default:
            assert(false);
        }
    }
  }
  if (stack.size() != 1) {
    throw ParseError("Invalid expression");
  }
  return std::move(stack.back());
}
