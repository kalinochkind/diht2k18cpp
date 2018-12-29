#include <map>
#include <functional>
#include "expression.h"

std::unique_ptr<Constant> zero() {
  return std::make_unique<Constant>(0);
}

std::unique_ptr<Constant> one() {
  return std::make_unique<Constant>(1);
}

expression_ptr_t multiply_by_k(expression_ptr_t expr, Rational k) {
  if (k == 0) {
    return zero();
  }
  if (k == 1) {
    return std::move(expr);
  }
  if (k == -1) {
    return std::make_unique<Negation>(std::move(expr))->optimize();
  }
  return std::make_unique<Multiplication>(std::make_unique<Constant>(k), std::move(expr))->optimize();
}

expression_ptr_t power_of_k(expression_ptr_t expr, expression_ptr_t k) {
  k = k->optimize();
  if (k->is_one()) {
    return std::move(expr);
  }
  if (k->is_zero()) {
    return one();
  }
  return std::make_unique<Power>(std::move(expr), std::move(k))->optimize();
}

std::pair<std::unique_ptr<Constant>, expression_ptr_t> extract_constant(expression_ptr_t e) {
  bool neg = false;
  auto rm_neg = e->remove_negation();
  if (rm_neg) {
    neg = true;
    e = std::move(rm_neg);
  }
  auto as_const = dynamic_cast<Constant*>(e.get());
  if (as_const) {
    return {std::make_unique<Constant>(neg ? -as_const->value : as_const->value), one()};
  }
  auto as_mul = dynamic_cast<Multiplication*>(e.get());
  if (!as_mul) {
    return {std::make_unique<Constant>(neg ? -1 : 1), std::move(e)};
  }
  as_const = dynamic_cast<Constant*>(as_mul->elements[0].get());
  if (!as_const) {
    return {std::make_unique<Constant>(neg ? -1 : 1), std::move(e)};
  }
  Rational val = as_const->value;
  if (neg) {
    val = -val;
  }
  as_mul->elements.erase(as_mul->elements.begin());
  return {std::make_unique<Constant>(val), e->optimize()};
}


std::pair<expression_ptr_t, expression_ptr_t> extract_exponent(expression_ptr_t e) {
  auto as_pow = dynamic_cast<Power*>(e.get());
  if (!as_pow) {
    return {one(), std::move(e)};
  }
  return {as_pow->power->clone(), as_pow->element->clone()};
}

std::vector<expression_ptr_t> merge_divisions(std::vector<expression_ptr_t> v) {
  for (size_t i = 0; i < v.size(); --i) {
    auto as_division = dynamic_cast<Division*>(v[i].get());
    if (!as_division) {
      continue;
    }
    for (ssize_t j = i + 1; j < v.size(); ++j) {
      auto as_division_right = dynamic_cast<Division*>(v[j].get());
      if (!as_division_right) {
        continue;
      }
      as_division->num = std::make_unique<Multiplication>(std::move(as_division->num),
                                                          std::move(as_division_right->num));
      as_division->den = std::make_unique<Multiplication>(std::move(as_division->den),
                                                          std::move(as_division_right->den));
      v.erase(v.begin() + j);
    }
    v[i] = v[i]->optimize();
    break;
  }
  return std::move(v);
}

template <class Op>
std::vector<expression_ptr_t> unwrap_operations(const std::vector<expression_ptr_t>& v) {
  std::vector<expression_ptr_t> res;
  for (const auto& el : v) {
    auto opt = el->optimize();
    auto as_op = dynamic_cast<Op*>(opt.get());
    if (as_op) {
      for (auto&& sub_el : as_op->elements) {
        res.push_back(std::move(sub_el));
      }
    } else {
      res.push_back(std::move(opt));
    }
  }
  return res;
}

template std::vector<expression_ptr_t> unwrap_operations<Addition>(const std::vector<expression_ptr_t>& v);

template std::vector<expression_ptr_t> unwrap_operations<Multiplication>(const std::vector<expression_ptr_t>& v);


const std::map<std::string, std::function<expression_ptr_t(const expression_ptr_t&)>> function_derivatives = {
    {"log",    [](const expression_ptr_t& x) -> expression_ptr_t {
      return std::make_unique<Division>(one(), x->clone());
    }},
    {"ln",     [](const expression_ptr_t& x) -> expression_ptr_t {
      return std::make_unique<Division>(one(), x->clone());
    }},
    {"sin",    [](const expression_ptr_t& x) -> expression_ptr_t {
      return std::make_unique<Function>("cos", x->clone());
    }},
    {"cos",    [](const expression_ptr_t& x) -> expression_ptr_t {
      return std::make_unique<Negation>(std::make_unique<Function>("sin", x->clone()));
    }},
    {"sqrt",   [](const expression_ptr_t& x) -> expression_ptr_t {
      return std::make_unique<Division>(one(), std::make_unique<Multiplication>(
          std::make_unique<Constant>(2), std::make_unique<Function>("sqrt", x->clone())
      ));
    }},
    {"tg",     [](const expression_ptr_t& x) -> expression_ptr_t {
      return std::make_unique<Division>(one(), std::make_unique<Power>(
          std::make_unique<Function>("cos", x->clone()), std::make_unique<Constant>(2)
      ));
    }},
    {"ctg",    [](const expression_ptr_t& x) -> expression_ptr_t {
      return std::make_unique<Negation>(
          std::make_unique<Division>(one(), std::make_unique<Power>(
              std::make_unique<Function>("sin", x->clone()), std::make_unique<Constant>(2)
          )));
    }},
    {"sh",     [](const expression_ptr_t& x) -> expression_ptr_t {
      return std::make_unique<Function>("ch", x->clone());
    }},
    {"ch",     [](const expression_ptr_t& x) -> expression_ptr_t {
      return std::make_unique<Function>("sh", x->clone());
    }},
    {"th",     [](const expression_ptr_t& x) -> expression_ptr_t {
      return std::make_unique<Division>(one(), std::make_unique<Power>(
          std::make_unique<Function>("ch", x->clone()), std::make_unique<Constant>(2)
      ));
    }},
    {"cth",    [](const expression_ptr_t& x) -> expression_ptr_t {
      return std::make_unique<Negation>(
          std::make_unique<Division>(one(), std::make_unique<Power>(
              std::make_unique<Function>("sh", x->clone()), std::make_unique<Constant>(2)
          )));
    }},
    {"arcsin", [](const expression_ptr_t& x) -> expression_ptr_t {
      return std::make_unique<Division>(one(), std::make_unique<Function>("sqrt", std::make_unique<Addition>(
          one(), std::make_unique<Negation>(std::make_unique<Power>(x->clone(), std::make_unique<Constant>(2)))
      )));
    }},
    {"arccos", [](const expression_ptr_t& x) -> expression_ptr_t {
      return std::make_unique<Division>(std::make_unique<Constant>(-1), std::make_unique<Function>(
          "sqrt", std::make_unique<Addition>(
              one(), std::make_unique<Negation>(std::make_unique<Power>(x->clone(), std::make_unique<Constant>(2)))
          )));
    }},
    {"arctg",  [](const expression_ptr_t& x) -> expression_ptr_t {
      return std::make_unique<Division>(one(), std::make_unique<Addition>(
          one(), std::make_unique<Power>(x->clone(), std::make_unique<Constant>(2))
      ));
    }},
    {"arcctg", [](const expression_ptr_t& x) -> expression_ptr_t {
      return std::make_unique<Division>(std::make_unique<Constant>(-1), std::make_unique<Addition>(
          one(), std::make_unique<Power>(x->clone(), std::make_unique<Constant>(2))
      ));
    }},
};


bool Expression::is_zero() const {
  return false;
}

bool Expression::is_one() const {
  return false;
}

expression_ptr_t Expression::optimize() const {
  return clone();
}

const std::string Expression::to_string(bool tex) const {
  std::string s;
  if (tex) {
    to_tex(s);
  } else {
    to_printable(s);
  }
  return s;
}

void Expression::to_tex(std::string& out, safety_t safety) const {
  out.push_back('{');
  to_tex_internal(out, safety);
  out.push_back('}');
}

expression_ptr_t Expression::remove_negation() const {
  return nullptr;
}

bool Expression::can_take_constant() const {
  return false;
}

expression_ptr_t Expression::take_constant(expression_ptr_t c) const {
  return nullptr;
}

bool Expression::can_be_taken() const {
  return true;
}

bool Constant::is_zero() const {
  return value == 0;
}

bool Constant::is_one() const {
  return value == 1;
}

expression_ptr_t Constant::clone() const {
  return std::make_unique<Constant>(value);
}

expression_ptr_t Constant::derivative(char var) const {
  return zero();
}

void Constant::to_tex_internal(std::string& out, safety_t safety) const {
  to_printable(out, safety);
}

void Constant::to_printable(std::string& out, safety_t safety) const {
  if (value >= 0) {
    out += value.asDecimal(6);
  } else {
    if (safety < safety_t::FULL) {
      out.push_back('(');
    }
    out += value.asDecimal(6);
    if (safety < safety_t::FULL) {
      out.push_back(')');
    }
  }
}

expression_ptr_t Constant::remove_negation() const {
  if (value < 0) {
    return std::make_unique<Constant>(-value);
  }
  return nullptr;
}


expression_ptr_t Variable::derivative(char var) const {
  if (var == name) {
    return one();
  }
  return zero();
}

expression_ptr_t Variable::clone() const {
  return std::make_unique<Variable>(name);
}

void Variable::to_tex_internal(std::string& out, safety_t safety) const {
  out.push_back(name);
}

void Variable::to_printable(std::string& out, safety_t safety) const {
  out.push_back(name);
}

expression_ptr_t Addition::derivative(char var) const {
  std::vector<expression_ptr_t> children_derivatives;
  children_derivatives.reserve(elements.size());
  for (const auto& el : elements) {
    children_derivatives.push_back(el->derivative(var));
  }
  return std::make_unique<Addition>(std::move(children_derivatives));
}

expression_ptr_t Addition::optimize() const {
  std::vector<expression_ptr_t> res;
  std::vector<expression_ptr_t> temp = unwrap_operations<Addition>(elements);
  std::map<std::string, Rational> coeffs;
  std::map<std::string, expression_ptr_t> atoms;
  std::vector<std::pair<std::unique_ptr<Constant>, expression_ptr_t>> pairs;
  for (auto&& p : temp) {
    auto pair = extract_constant(std::move(p));
    std::string pair_string = pair.second->to_string();
    coeffs[pair_string] += pair.first->value;
    if (!atoms.count(pair_string)) {
      atoms[pair_string] = std::move(pair.second);
    }
  }
  for (auto&& kv : atoms) {
    auto mul = multiply_by_k(std::move(kv.second), coeffs[kv.first]);
    if (!mul->is_zero()) {
      res.push_back(std::move(mul));
    }
  }
  if (res.empty()) {
    return zero();
  }
  if (res.size() == 1) {
    return std::move(res[0]);
  }
  if (res[0]->remove_negation()) {
    for (size_t i = 1; i < res.size(); ++i) {
      if (!res[i]->remove_negation()) {
        std::swap(res[0], res[i]);
        break;
      }
    }
  }
  return std::make_unique<Addition>(std::move(res));
}

expression_ptr_t Addition::clone() const {
  std::vector<expression_ptr_t> clones;
  clones.reserve(elements.size());
  for (const auto& e : elements) {
    clones.push_back(e->clone());
  }
  return std::make_unique<Addition>(std::move(clones));
}

void Addition::to_printable(std::string& out, safety_t safety) const {
  bool first_time = true;
  if (safety < safety_t::FULL) {
    out.push_back('(');
  }
  for (const auto& e : elements) {
    auto neg = e->remove_negation();
    if (neg) {
      if (first_time) {
        out.push_back('-');
        first_time = false;
      } else {
        out += " - ";
      }
      neg->to_printable(out, safety_t::ADD);
    } else {
      if (!first_time) {
        out += " + ";
      }
      first_time = false;
      e->to_printable(out, safety_t::ADD);
    }
  }
  if (safety < safety_t::FULL) {
    out.push_back(')');
  }
}

void Addition::to_tex_internal(std::string& out, safety_t safety) const {
  bool first_time = true;
  if (safety < safety_t::FULL) {
    out += "\\left(";
  }
  for (const auto& e : elements) {
    auto neg = e->remove_negation();
    if (neg) {
      out.push_back('-');
      first_time = false;
      neg->to_tex(out, safety_t::ADD);
    } else {
      if (!first_time) {
        out.push_back('+');
      }
      first_time = false;
      e->to_tex(out, safety_t::ADD);
    }
  }
  if (safety < safety_t::FULL) {
    out += "\\right)";
  }
}

bool Addition::can_be_taken() const {
  for (const auto& el : elements) {
    if (!el->can_be_taken()) {
      return false;
    }
  }
  return true;
}

expression_ptr_t Multiplication::derivative(char var) const {
  std::vector<expression_ptr_t> products;
  products.reserve(elements.size());
  for (size_t i = 0; i < elements.size(); ++i) {
    std::vector<expression_ptr_t> items(elements.size());
    items[i] = elements[i]->derivative(var);
    for (size_t j = 0; j < elements.size(); ++j) {
      if (i != j) {
        items[j] = elements[j]->clone();
      }
    }
    products.emplace_back(std::make_unique<Multiplication>(std::move(items)));
  }
  return std::make_unique<Addition>(std::move(products));
}

expression_ptr_t Multiplication::optimize() const {
  std::vector<expression_ptr_t> res;
  std::vector<expression_ptr_t> temp = unwrap_operations<Multiplication>(elements);
  std::map<std::string, expression_ptr_t> coeffs;
  std::map<std::string, expression_ptr_t> atoms;
  Rational constant = 1;
  for (auto&& opt : temp) {
    if (opt->is_zero()) {
      return zero();
    }
    if (opt->is_one()) {
      continue;
    }
    auto rm_neg = opt->remove_negation();
    if (rm_neg) {
      constant *= -1;
      opt = std::move(rm_neg);
    }
    auto as_const = dynamic_cast<Constant*>(opt.get());
    if (as_const) {
      constant *= as_const->value;
      continue;
    }

    auto pair = extract_exponent(std::move(opt));
    std::string pair_string = pair.second->to_string();
    if (coeffs.count(pair_string)) {
      coeffs[pair_string] = std::make_unique<Addition>(std::move(coeffs[pair_string]), std::move(pair.first));
    } else {
      coeffs[pair_string] = std::move(pair.first);
    }
    if (!atoms.count(pair_string)) {
      atoms[pair_string] = std::move(pair.second);
    }
  }
  std::vector<expression_ptr_t> cur;
  std::string cur_string;
  std::string el_string;
  for (auto&& el : res) {
    el_string = el->to_string();
    if (cur.empty() || cur_string == el_string) {
      cur_string = el_string;
      cur.push_back(std::move(el));
      continue;
    }
    temp.push_back(power_of_k(std::move(cur[0]), std::make_unique<Constant>(cur.size())));
    cur_string = el_string;
    cur.clear();
    cur.push_back(std::move(el));
  }
  for (auto&& kv : atoms) {
    auto mul = power_of_k(std::move(kv.second), std::move(coeffs[kv.first]));
    if (!mul->is_one()) {
      res.push_back(std::move(mul));
    }
  }
  if (constant != 1 && constant != -1) {
    bool taken = false;
    for (auto& el : res) {
      if (!el->can_take_constant()) {
        continue;
      }
      el = el->take_constant(std::make_unique<Constant>(constant));
      taken = true;
      break;
    }
    if (!taken) {
      res.insert(res.begin(), std::make_unique<Constant>(constant));
    }
  }
  bool negate = (constant == -1);
  if (res.empty()) {
    return std::make_unique<Constant>(negate ? -1 : 1);
  }

  while (true) {
    bool success = false;
    for (size_t i = 0; i < res.size(); ++i) {
      if (!res[i]->can_take_constant()) {
        continue;
      }
      for (size_t j = 0; j < res.size(); ++j) {
        if (i == j || !res[j]->can_be_taken()) {
          continue;
        }
        res[i] = res[i]->take_constant(std::move(res[j]));
        res.erase(res.begin() + j);
        success = true;
        break;
      }
      break;
    }
    if (!success) {
      break;
    }
  }

  res = merge_divisions(std::move(res));

  if (res.size() == 1) {
    if (negate) {
      return multiply_by_k(std::move(res[0]), -1);
    }
    return std::move(res[0]);
  }
  if (negate) {
    return multiply_by_k(std::make_unique<Multiplication>(std::move(res)), -1);
  }
  return std::make_unique<Multiplication>(std::move(res));
}

expression_ptr_t Multiplication::clone() const {
  std::vector<expression_ptr_t> clones;
  clones.reserve(elements.size());
  for (const auto& e : elements) {
    clones.push_back(e->clone());
  }
  return std::make_unique<Multiplication>(std::move(clones));
}

void Multiplication::to_printable(std::string& out, safety_t safety) const {
  bool first_time = true;
  if (safety < safety_t::ADD) {
    out.push_back('(');
  }
  for (const auto& e : elements) {
    if (!first_time) {
      out += "*";
    }
    first_time = false;
    e->to_printable(out, safety_t::MUL);
  }
  if (safety < safety_t::ADD) {
    out.push_back(')');
  }
}

void Multiplication::to_tex_internal(std::string& out, safety_t safety) const {
  bool first_time = true;
  if (safety < safety_t::ADD) {
    out += "\\left(";
  }
  for (const auto& e : elements) {
    if (!first_time) {
      out += "\\cdot ";
    }
    first_time = false;
    e->to_tex(out, safety_t::MUL);
  }
  if (safety < safety_t::ADD) {
    out += "\\right)";
  }
}

expression_ptr_t Multiplication::remove_negation() const {
  auto rmneg = elements[0]->remove_negation();
  if (!rmneg) {
    return nullptr;
  }
  std::vector<expression_ptr_t> res;
  for (const auto& el : elements) {
    res.push_back(el->clone());
  }
  res[0] = std::move(rmneg);
  return std::make_unique<Multiplication>(std::move(res));
}

bool Multiplication::can_be_taken() const {
  for (const auto& el : elements) {
    if (!el->can_be_taken()) {
      return false;
    }
  }
  return true;
}

expression_ptr_t Negation::derivative(char var) const {
  return std::make_unique<Negation>(element->derivative(var));
}

expression_ptr_t Negation::optimize() const {
  auto opt = element->optimize();
  if (opt->is_zero()) {
    return zero();
  }
  auto as_const = dynamic_cast<Constant*>(opt.get());
  if (as_const) {
    return std::make_unique<Constant>(-as_const->value);
  }
  auto neg = opt->remove_negation();
  if (neg) {
    return std::move(neg);
  }
  return std::make_unique<Negation>(std::move(opt));
}

expression_ptr_t Negation::clone() const {
  return std::make_unique<Negation>(element->clone());
}

void Negation::to_printable(std::string& out, safety_t safety) const {
  if (safety < safety_t::FULL) {
    out.push_back('(');
  }
  out.push_back('-');
  element->to_printable(out, safety_t::ADD);
  if (safety < safety_t::FULL) {
    out.push_back(')');
  }
}

void Negation::to_tex_internal(std::string& out, safety_t safety) const {
  if (safety < safety_t::FULL) {
    out += "\\left(-";
  }
  element->to_tex(out, safety_t::ADD);
  if (safety < safety_t::FULL) {
    out += "\\right)";
  }
}

expression_ptr_t Negation::remove_negation() const {
  return element->clone();
}

bool Negation::can_be_taken() const {
  return element->can_be_taken();
}

expression_ptr_t Function::derivative(char var) const {
  return std::make_unique<Multiplication>(
      function_derivatives.at(fun)(arg),
      arg->derivative(var)
  );
}

expression_ptr_t Function::optimize() const {
  return std::make_unique<Function>(fun, arg->optimize());
}

expression_ptr_t Function::clone() const {
  return std::make_unique<Function>(fun, arg->clone());
}

void Function::to_printable(std::string& out, safety_t safety) const {
  out += fun;
  out.push_back('(');
  arg->to_printable(out, safety_t::FULL);
  out.push_back(')');
}

void Function::to_tex_internal(std::string& out, safety_t safety) const {
  if (fun == "sqrt") {
    out += "\\sqrt";
    arg->to_tex(out, safety_t::FULL);
    return;
  }
  out += "\\mathrm{";
  out += fun;
  out += "}\\left(";
  arg->to_tex(out, safety_t::FULL);
  out += "\\right)";
}

expression_ptr_t Power::derivative(char var) const {
  auto power_derivative = power->derivative(var)->optimize();
  auto main = std::make_unique<Multiplication>(
      power->clone(),
      std::make_unique<Power>(element->clone(),
                              std::make_unique<Addition>(power->clone(), std::make_unique<Constant>(-1))),
      element->derivative(var)
  );
  if (power_derivative->is_zero()) {
    return main;
  }
  auto second = std::make_unique<Multiplication>(
      clone(),
      std::move(power_derivative),
      std::make_unique<Function>("log", element->clone())
  );
  return std::make_unique<Addition>(std::move(main), std::move(second));
}

expression_ptr_t Power::optimize() const {
  auto elem_opt = element->optimize();
  auto pow_opt = power->optimize();
  if (pow_opt->is_zero() || elem_opt->is_one()) {
    return one();
  }
  if (pow_opt->is_one()) {
    return std::move(elem_opt);
  }
  if (elem_opt->is_zero()) {
    return zero();
  }
  return std::make_unique<Power>(std::move(elem_opt), std::move(pow_opt));
}

expression_ptr_t Power::clone() const {
  return std::make_unique<Power>(element->clone(), power->clone());
}

void Power::to_printable(std::string& out, safety_t safety) const {
  if (safety < safety_t::MUL) {
    out.push_back('(');
  }
  element->to_printable(out, safety_t::NONE);
  out.push_back('^');
  power->to_printable(out, safety_t::NONE);
  if (safety < safety_t::MUL) {
    out.push_back(')');
  }
}

void Power::to_tex_internal(std::string& out, safety_t safety) const {
  element->to_tex(out, safety_t::NONE);
  out.push_back('^');
  power->to_tex(out, safety_t::FULL);
}

bool Power::can_be_taken() const {
  return element->can_be_taken();
}

expression_ptr_t Division::derivative(char var) const {
  auto den_deriv = den->derivative(var)->optimize();
  if (den_deriv->is_zero()) {
    return std::make_unique<Division>(num->derivative(var), den->clone());
  }
  auto numerator = std::make_unique<Addition>(
      std::make_unique<Multiplication>(num->derivative(var), den->clone()),
      std::make_unique<Negation>(std::make_unique<Multiplication>(num->clone(), std::move(den_deriv)))
  );
  return std::make_unique<Division>(std::move(numerator),
                                    std::make_unique<Power>(den->clone(), std::make_unique<Constant>(2)));
}

expression_ptr_t Division::optimize() const {
  auto num_opt = num->optimize();
  auto den_opt = den->optimize();
  if (num_opt->is_zero()) {
    return zero();
  }
  if (den_opt->is_one()) {
    return std::move(num_opt);
  }
  auto res = std::make_unique<Division>(std::move(num_opt), std::move(den_opt));
  auto num_extr = extract_constant(res->num->clone());
  auto den_extr = extract_constant(res->den->clone());
  auto fair_const = num_extr.first->value / den_extr.first->value;
  res = std::make_unique<Division>(
      std::make_unique<Multiplication>(std::make_unique<Constant>(fair_const.numerator()),
                                       std::move(num_extr.second))->optimize(),
      std::make_unique<Multiplication>(std::make_unique<Constant>(fair_const.denominator()),
                                       std::move(den_extr.second))->optimize());
  if (res->den->is_one()) {
    return std::move(res->num);
  }
  auto rmneg = res->remove_negation();
  if (rmneg) {
    rmneg = rmneg->remove_negation();
    if (rmneg) {
      return rmneg;
    }
  }
  return res;
}

expression_ptr_t Division::clone() const {
  return std::make_unique<Division>(num->clone(), den->clone());
}

void Division::to_printable(std::string& out, safety_t safety) const {
  if (safety < safety_t::NONE) {
    out.push_back('(');
  }
  num->to_printable(out, safety_t::NONE);
  out.push_back('/');
  den->to_printable(out, safety_t::NONE);
  if (safety < safety_t::NONE) {
    out.push_back(')');
  }
}

void Division::to_tex_internal(std::string& out, safety_t safety) const {
  out += "\\frac{";
  num->to_tex(out, safety_t::FULL);
  out += "}{";
  den->to_tex(out, safety_t::FULL);
  out.push_back('}');
}

expression_ptr_t Division::remove_negation() const {
  auto num_neg = num->remove_negation();
  if (num_neg) {
    return std::make_unique<Division>(std::move(num_neg), den->clone());
  }
  auto den_neg = den->remove_negation();
  if (den_neg) {
    return std::make_unique<Division>(num->clone(), std::move(den_neg));
  }
  return nullptr;
}

bool Division::can_take_constant() const {
  auto as_const = dynamic_cast<Constant*>(num.get());
  return as_const;
}

expression_ptr_t Division::take_constant(expression_ptr_t c) const {
  c = std::make_unique<Multiplication>(num->clone(), std::move(c));
  return std::make_unique<Division>(std::move(c), den->clone())->optimize();
}

bool Division::can_be_taken() const {
  return false;
}
