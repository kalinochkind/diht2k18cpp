#pragma once

#include <vector>
#include <memory>
#include "rational.h"


struct Expression;

using expression_ptr_t = std::unique_ptr<Expression>;

inline void fill_vector(std::vector<expression_ptr_t>& v) {
}

template <class T, class ...Tail>
inline void fill_vector(std::vector<expression_ptr_t>& v, T&& a, Tail&& ...rem) {
  v.emplace_back(std::forward<T>(a));
  fill_vector(v, std::forward<Tail>(rem)...);
}

enum class safety_t {
  NONE, MUL, ADD, FULL
};


struct Expression {

  virtual expression_ptr_t derivative(char var) const = 0;

  virtual bool is_zero() const;

  virtual bool is_one() const;

  virtual expression_ptr_t optimize() const;

  virtual expression_ptr_t clone() const = 0;

  virtual void to_printable(std::string& out, safety_t safety = safety_t::FULL) const = 0;

  const std::string to_string(bool tex = false) const;

  void to_tex(std::string& out, safety_t safety = safety_t::FULL) const;

  virtual void to_tex_internal(std::string& out, safety_t safety) const = 0;

  virtual expression_ptr_t remove_negation() const;

  virtual bool can_take_constant() const;

  virtual expression_ptr_t take_constant(expression_ptr_t c) const;

  virtual bool can_be_taken() const;

  virtual ~Expression() = default;
};

using expression_ptr_t = std::unique_ptr<Expression>;

struct Constant : public Expression {

  explicit Constant(Rational value)
      : value(std::move(value)) {
  };

  bool is_zero() const override;

  bool is_one() const override;

  expression_ptr_t clone() const override;

  expression_ptr_t derivative(char var) const override;

  void to_tex_internal(std::string& out, safety_t safety = safety_t::FULL) const override;

  void to_printable(std::string& out, safety_t safety = safety_t::FULL) const override;

  expression_ptr_t remove_negation() const override;

  Rational value;

};

struct Variable : public Expression {
  explicit Variable(char name)
      : name(name) {
  };

  expression_ptr_t derivative(char var) const override;

  expression_ptr_t clone() const override;

  void to_tex_internal(std::string& out, safety_t safety = safety_t::FULL) const override;

  void to_printable(std::string& out, safety_t safety = safety_t::FULL) const override;

  char name;
};

struct Addition : public Expression {
  explicit Addition(std::vector<expression_ptr_t> elements)
      : elements(std::move(elements)) {
  }

  template <class ...Args>
  explicit Addition(Args&& ...args)
      : elements() {
    fill_vector<Args...>(elements, std::forward<Args>(args)...);
  };

  expression_ptr_t derivative(char var) const override;

  expression_ptr_t optimize() const override;

  expression_ptr_t clone() const override;

  void to_printable(std::string& out, safety_t safety = safety_t::FULL) const override;

  void to_tex_internal(std::string& out, safety_t safety = safety_t::FULL) const override;

  bool can_be_taken() const override;

  std::vector<expression_ptr_t> elements;
};


struct Multiplication : public Expression {
  explicit Multiplication(std::vector<expression_ptr_t> elements)
      : elements(std::move(elements)) {
  }

  template <class ...Args>
  explicit Multiplication(Args&& ...args)
      : elements() {
    fill_vector<Args...>(elements, std::forward<Args>(args)...);
  };

  expression_ptr_t derivative(char var) const override;

  expression_ptr_t optimize() const override;

  expression_ptr_t clone() const override;

  void to_printable(std::string& out, safety_t safety = safety_t::FULL) const override;

  void to_tex_internal(std::string& out, safety_t safety = safety_t::FULL) const override;

  expression_ptr_t remove_negation() const override;

  bool can_be_taken() const override;

  std::vector<expression_ptr_t> elements;
};

struct Negation : public Expression {
  explicit Negation(expression_ptr_t element)
      : element(std::move(element)) {
  }

  expression_ptr_t derivative(char var) const override;

  expression_ptr_t optimize() const override;

  expression_ptr_t clone() const override;

  void to_printable(std::string& out, safety_t safety = safety_t::FULL) const override;

  void to_tex_internal(std::string& out, safety_t safety = safety_t::FULL) const override;

  expression_ptr_t remove_negation() const override;

  bool can_be_taken() const override;

  expression_ptr_t element;
};

struct Function : public Expression {
  explicit Function(std::string fun, expression_ptr_t arg)
      : fun(std::move(fun)), arg(std::move(arg)) {
  }

  expression_ptr_t derivative(char var) const override;

  expression_ptr_t optimize() const override;

  expression_ptr_t clone() const override;

  void to_printable(std::string& out, safety_t safety = safety_t::FULL) const override;

  void to_tex_internal(std::string& out, safety_t safety = safety_t::FULL) const override;

  std::string fun;
  expression_ptr_t arg;
};


struct Power : public Expression {
  explicit Power(expression_ptr_t element, expression_ptr_t power)
      : element(std::move(element)), power(std::move(power)) {
  }

  expression_ptr_t derivative(char var) const override;

  expression_ptr_t optimize() const override;

  expression_ptr_t clone() const override;

  void to_printable(std::string& out, safety_t safety = safety_t::FULL) const override;

  void to_tex_internal(std::string& out, safety_t safety = safety_t::FULL) const override;

  bool can_be_taken() const override;

  expression_ptr_t element;
  expression_ptr_t power;
};


struct Division : public Expression {
  explicit Division(expression_ptr_t num, expression_ptr_t den)
      : num(std::move(num)), den(std::move(den)) {
  }

  expression_ptr_t derivative(char var) const override;

  expression_ptr_t optimize() const override;

  expression_ptr_t clone() const override;

  void to_printable(std::string& out, safety_t safety = safety_t::FULL) const override;

  void to_tex_internal(std::string& out, safety_t safety = safety_t::FULL) const override;

  expression_ptr_t remove_negation() const override;

  bool can_take_constant() const override;

  expression_ptr_t take_constant(expression_ptr_t c) const override;

  bool can_be_taken() const override;

  expression_ptr_t num;
  expression_ptr_t den;
};
