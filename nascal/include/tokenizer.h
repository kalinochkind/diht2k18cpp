#pragma once
#include <string>
#include <stdexcept>


enum class token_type_t {
  NONE, INT, UNARY_OP, ADD_OP, MUL_OP, CMP_OP, SEMICOLON, NAME, LEFT_PAR, RIGHT_PAR, COMMA, ASSIGN,
  END, RETURN, DEF, IF, THEN, ELSE, WHILE, DO, BREAK, CONTINUE, FOR
};

struct Token {
  token_type_t type{token_type_t::NONE};
  std::string value;
};


class parse_error : public std::runtime_error {
  using std::runtime_error::runtime_error;
};


class Tokenizer {
 public:
  explicit Tokenizer(std::string s)
      : s(std::move(s)) {
  }

  Token get_token() {
    if (has_token) {
      has_token = false;
      return last;
    }
    return next_token();
  }

  Token peek_token() {
    if (!has_token) {
      last = next_token();
      has_token = true;
    }
    return last;
  }

  size_t get_line() const {
    return oldline;
  }

  size_t get_linepos() const {
    return oldlinepos;
  }

 private:
  Token next_token() {
    oldline = line;
    oldlinepos = linepos;
    Token t{};
    skip_whitespace();
    if (pos >= s.size()) {
      return t;
    }
    if (isdigit(s[pos])) {
      t.type = token_type_t::INT;
      do {
        t.value.push_back(s[pos++]);
        ++linepos;
      } while (pos < s.size() && isdigit(s[pos]));
      expecting_binary_op = true;
      return t;
    }
    if (s[pos] == '\'') {
      t.type = token_type_t::INT;
      expecting_binary_op = true;
      if (pos + 2 >= s.size()) {
        throw parse_error("Unexpected end of file");
      }
      if (s[pos + 2] != '\'') {
        throw parse_error("Invalid character");
      }
      t.value = std::to_string(static_cast<unsigned>(s[pos + 1]));
      pos += 3;
      return t;
    }
    if (s[pos] == '~' || s[pos] == '!' || s[pos] == '$' || s[pos] == '@' || (s[pos] == '-' && !expecting_binary_op)) {
      t.type = token_type_t::UNARY_OP;
      t.value = std::string(1, s[pos++]);
      ++linepos;
      expecting_binary_op = false;
      return t;
    }
    if (s[pos] == '+' || s[pos] == '-' || s[pos] == '|' || s[pos] == '^') {
      t.type = token_type_t::ADD_OP;
      t.value = std::string(1, s[pos++]);
      ++linepos;
      expecting_binary_op = false;
      return t;
    }
    if (s[pos] == '*' || s[pos] == '/' || s[pos] == '%' || s[pos] == '&') {
      t.type = token_type_t::MUL_OP;
      t.value = std::string(1, s[pos++]);
      ++linepos;
      expecting_binary_op = false;
      return t;
    }
    if (s[pos] == '=' || s[pos] == '>' || s[pos] == '<') {
      t.type = token_type_t::CMP_OP;
      t.value = std::string(1, s[pos++]);
      ++linepos;
      expecting_binary_op = false;
      return t;
    }
    if (s[pos] == ';') {
      ++pos;
      ++linepos;
      t.type = token_type_t::SEMICOLON;
      expecting_binary_op = false;
      return t;
    }
    if (isalpha(s[pos]) || s[pos] == '_') {
      do {
        t.value.push_back(s[pos++]);
        ++linepos;
      } while (pos < s.size() && (isalnum(s[pos]) || s[pos] == '_'));
      expecting_binary_op = false;
      if (t.value == "END") {
        t.type = token_type_t::END;
      } else if (t.value == "RETURN") {
        t.type = token_type_t::RETURN;
      } else if (t.value == "DEF") {
        t.type = token_type_t::DEF;
      } else if (t.value == "IF") {
        t.type = token_type_t::IF;
      } else if (t.value == "THEN") {
        t.type = token_type_t::THEN;
      } else if (t.value == "ELSE") {
        t.type = token_type_t::ELSE;
      } else if (t.value == "WHILE") {
        t.type = token_type_t::WHILE;
      } else if (t.value == "DO") {
        t.type = token_type_t::DO;
      } else if (t.value == "BREAK") {
        t.type = token_type_t::BREAK;
      } else if (t.value == "CONTINUE") {
        t.type = token_type_t::CONTINUE;
      } else if (t.value == "FOR") {
        t.type = token_type_t::FOR;
      } else {
        expecting_binary_op = true;
        t.type = token_type_t::NAME;
      }
      return t;
    }
    if (s[pos] == '(') {
      ++pos;
      ++linepos;
      t.type = token_type_t::LEFT_PAR;
      expecting_binary_op = false;
      return t;
    }
    if (s[pos] == ')') {
      ++pos;
      ++linepos;
      t.type = token_type_t::RIGHT_PAR;
      expecting_binary_op = true;
      return t;
    }
    if (s[pos] == ',') {
      ++pos;
      ++linepos;
      t.type = token_type_t::COMMA;
      expecting_binary_op = false;
      return t;
    }
    if (s[pos] == ':' && pos + 1 < s.size() && s[pos + 1] == '=') {
      pos += 2;
      linepos += 2;
      t.type = token_type_t::ASSIGN;
      expecting_binary_op = false;
      return t;
    }
    throw parse_error(std::string("Unexpected symbol: ") + s[pos]);
  }


  void skip_whitespace() {
    while (true) {
      while (pos < s.size() && isspace(s[pos])) {
        ++pos;
        ++linepos;
        if (s[pos - 1] == '\n') {
          linepos = 1;
          ++line;
        }
      }
      if (pos < s.size() && s[pos] == '#') {
        while (pos < s.size() && s[pos] != '\n') {
          ++pos;
          ++linepos;
        }
      } else {
        break;
      }
    }
  }

  std::string s;
  size_t pos{0};
  bool expecting_binary_op{false};
  Token last;
  bool has_token{false};
  size_t line{1}, linepos{1};
  size_t oldline{1}, oldlinepos{1};
};