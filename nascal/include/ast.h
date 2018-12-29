#pragma once
#include <string>
#include <vector>
#include <set>
#include <map>
#include <memory>
#include <cassert>


class compile_error : public std::runtime_error {
  using std::runtime_error::runtime_error;
};


struct CompilationContext {
  std::vector<std::string> code;
  std::map<std::string, size_t> offsets;
  size_t extra_offset;
  size_t local_count;
  std::string loop_start_label, loop_end_label;
};

using offsets_t = std::map<std::string, size_t>;

struct ASTNode {
  virtual void assemble(CompilationContext& c) const = 0;

  virtual ~ASTNode() {
  };
};

struct Expression {

  virtual void assemble(CompilationContext& c, uint8_t out) const = 0;

  virtual void get_address(CompilationContext& c, uint8_t out) const {
    throw compile_error("Taking address of rvalue");
  }

  virtual ~Expression() {
  };
};

using node_ptr_t = std::unique_ptr<ASTNode>;
using expr_ptr_t = std::unique_ptr<Expression>;


uint8_t another_reg(uint8_t reg, uint8_t offset = 1) {
  return (static_cast<size_t>(reg) + offset - 1) % 250u + 1;
}

std::string reg_name(uint8_t reg) {
  return "R" + std::to_string(reg);
}

struct NameExpression : public Expression {
  std::string name;

  void assemble(CompilationContext& c, uint8_t out) const override {
    if (!out) {
      return;
    }
    size_t offset = c.offsets.at(name) + c.extra_offset;
    if (offset) {
      c.code.push_back("set " + reg_name(out) + ' ' + std::to_string(offset * 4));
      c.code.push_back("add " + reg_name(out) + " RS");
      c.code.push_back("load32 " + reg_name(out) + ' ' + reg_name(out));
    } else {
      c.code.push_back("load32 " + reg_name(out) + " RS");
    }
  }

  void get_address(CompilationContext &c, uint8_t out) const override {
    if (!out) {
      return;
    }
    size_t offset = c.offsets.at(name) + c.extra_offset;
    if (offset) {
      c.code.push_back("set " + reg_name(out) + ' ' + std::to_string(offset * 4));
      c.code.push_back("add " + reg_name(out) + " RS");
    } else {
      c.code.push_back("mov " + reg_name(out) + " RS");
    }
  }
};

struct BinExpression : public Expression {
  expr_ptr_t left{nullptr};
  expr_ptr_t right{nullptr};
  char op{0};

  void assemble(CompilationContext& c, uint8_t out) const override {
    if (!out) {
      right->assemble(c, 0);
      left->assemble(c, 0);
      return;
    }
    right->assemble(c, out);
    c.code.push_back("push " + reg_name(out));
    ++c.extra_offset;
    left->assemble(c, out);
    c.code.push_back("pop " + reg_name(another_reg(out)));
    --c.extra_offset;
    std::string label_base = "@l" + std::to_string(c.code.size());
    switch (op) {
      case '=':
        c.code.push_back("xor " + reg_name(another_reg(out)) + ' ' + reg_name(out));
        c.code.push_back("set " + reg_name(out) + " 1");
        c.code.push_back("jiz " + label_base);
        c.code.push_back("xor " + reg_name(out) + ' ' + reg_name(out));
        c.code.push_back(label_base);
        break;
      case '<':
        c.code.push_back("sub " + reg_name(out) + ' ' + reg_name(another_reg(out)));
        c.code.push_back("set " + reg_name(out) + " 1");
        c.code.push_back("jis " + label_base);
        c.code.push_back("xor " + reg_name(out) + ' ' + reg_name(out));
        c.code.push_back(label_base);
        break;
      case '>':
        c.code.push_back("sub " + reg_name(another_reg(out)) + ' ' + reg_name(out));
        c.code.push_back("set " + reg_name(out) + " 1");
        c.code.push_back("jis " + label_base);
        c.code.push_back("xor " + reg_name(out) + ' ' + reg_name(out));
        c.code.push_back(label_base);
        break;
      case '+':
        c.code.push_back("add " + reg_name(out) + " " + reg_name(another_reg(out)));
        break;
      case '-':
        c.code.push_back("sub " + reg_name(out) + " " + reg_name(another_reg(out)));
        break;
      case '|':
        c.code.push_back("or " + reg_name(out) + " " + reg_name(another_reg(out)));
        break;
      case '^':
        c.code.push_back("xor " + reg_name(out) + " " + reg_name(another_reg(out)));
        break;
      case '*':
        c.code.push_back("smul " + reg_name(out) + " " + reg_name(another_reg(out)));
        break;
      case '/':
        c.code.push_back("sdiv " + reg_name(out) + " " + reg_name(another_reg(out)));
        break;
      case '%':
        c.code.push_back("smod " + reg_name(out) + " " + reg_name(another_reg(out)));
        break;
      case '&':
        c.code.push_back("and " + reg_name(out) + " " + reg_name(another_reg(out)));
        break;
      default:
        assert(false);
    }
  }
};

struct UnaryExpression : public Expression {
  expr_ptr_t operand{nullptr};
  char op{0};

  void assemble(CompilationContext& c, uint8_t out) const override {
    if (op == '@') {
      operand->get_address(c, out);
      return;
    }
    operand->assemble(c, out);
    if (!out) {
      return;
    }
    std::string label_base = "@l" + std::to_string(c.code.size());
    switch (op) {
      case '-':
        c.code.push_back("neg " + reg_name(out));
        break;
      case '~':
        c.code.push_back("not " + reg_name(out));
        break;
      case '!':
        c.code.push_back("and " + reg_name(out) + ' ' + reg_name(out));
        c.code.push_back("set " + reg_name(out) + " 1");
        c.code.push_back("jiz " + label_base);
        c.code.push_back("xor " + reg_name(out) + ' ' + reg_name(out));
        c.code.push_back(label_base);
        break;
      case '$':
        c.code.push_back("load32 " + reg_name(out) + ' ' + reg_name(out));
        break;
      default:
        assert(false);
    }
  }

  void get_address(CompilationContext &c, uint8_t out) const override {
    if (op == '$') {
      operand->assemble(c, out);
      return;
    }
    Expression::get_address(c, out);
  }
};

struct IntExpression : public Expression {
  int32_t value{0};

  void assemble(CompilationContext& c, uint8_t out) const override {
    if (!out) {
      return;
    }
    c.code.push_back("set " + reg_name(out) + ' ' + std::to_string(value));
  }
};

struct CallExpression : public Expression {
  std::string func;
  std::vector<expr_ptr_t> args;

  void assemble(CompilationContext& c, uint8_t out) const override {
    for (const auto& arg : args) {
      arg->assemble(c, another_reg(out));
      c.code.push_back("push " + reg_name(another_reg(out)));
      ++c.extra_offset;
    }
    c.code.push_back("call @func_" + func + '_' + std::to_string(args.size()));
    if (out) {
      c.code.push_back("mov " + reg_name(out) + " R0");
    }
    if (!args.empty()) {
      c.code.push_back("set R0 " + std::to_string(args.size() * 4));
      c.code.emplace_back("add RS R0");
      c.extra_offset -= args.size();
    }
  }
};


struct IfNode : public ASTNode {
  expr_ptr_t condition{nullptr};
  std::vector<node_ptr_t> body{};
  std::vector<node_ptr_t> else_body{};

  void assemble(CompilationContext& c) const override {
    condition->assemble(c, 1);
    std::string label_end = "@endif" + std::to_string(c.code.size());
    std::string label_else = "@else" + std::to_string(c.code.size());
    c.code.emplace_back("and R1 R1");
    if (!else_body.empty()) {
      c.code.push_back("jiz " + label_else);
      for (const auto& o : body) {
        o->assemble(c);
      }
      c.code.push_back("jmp " + label_end);
      c.code.push_back(label_else);
      for (const auto& o : else_body) {
        o->assemble(c);
      }
      c.code.push_back(label_end);
    } else {
      c.code.push_back("jiz " + label_end);
      for (const auto& o : body) {
        o->assemble(c);
      }
      c.code.push_back(label_end);
    }
  }
};

struct WhileNode : public ASTNode {
  expr_ptr_t condition{nullptr};
  std::vector<node_ptr_t> body{};

  void assemble(CompilationContext& c) const override {
    auto old_start = c.loop_start_label;
    auto old_end = c.loop_end_label;
    c.loop_end_label = "@endloop" + std::to_string(c.code.size());
    c.loop_start_label = "@loop" + std::to_string(c.code.size());
    c.code.emplace_back(c.loop_start_label);
    condition->assemble(c, 2);
    c.code.emplace_back("and R2 R2");
    c.code.emplace_back("jiz " + c.loop_end_label);
    for (const auto& o : body) {
      o->assemble(c);
    }
    c.code.push_back("jmp " + c.loop_start_label);
    c.code.push_back(c.loop_end_label);
    c.loop_start_label = old_start;
    c.loop_end_label = old_end;
  }
};

struct ForNode: public ASTNode {
  expr_ptr_t var{};
  expr_ptr_t num_start{nullptr};
  expr_ptr_t num_end{nullptr};
  expr_ptr_t num_step{nullptr};
  std::vector<node_ptr_t> body{};

  void assemble(CompilationContext& c) const override {
    auto old_start = c.loop_start_label;
    auto old_end = c.loop_end_label;
    c.loop_end_label = "@endfor" + std::to_string(c.code.size());
    c.loop_start_label = "@incfor" + std::to_string(c.code.size());
    auto loop_real_start = "@for" + std::to_string(c.code.size());
    var->get_address(c, 7);
    c.code.emplace_back("push R7");
    ++c.extra_offset;
    num_start->assemble(c, 8);
    c.code.emplace_back("load32 R7 RS");
    c.code.emplace_back("store32 R7 R8");
    c.code.push_back(loop_real_start);

    num_end->assemble(c, 8);
    c.code.emplace_back("and R8 R8");
    c.code.push_back("jiz " + c.loop_end_label);
    for (const auto& o : body) {
      o->assemble(c);
    }

    c.code.push_back(c.loop_start_label);
    num_step->assemble(c, 8);
    c.code.emplace_back("load32 R7 RS");
    c.code.emplace_back("load32 R9 R7");
    c.code.emplace_back("add R9 R8");
    c.code.emplace_back("store32 R7 R9");
    c.code.push_back("jmp " + loop_real_start);
    c.code.push_back(c.loop_end_label);
    c.code.emplace_back("pop R0");
    --c.extra_offset;
    c.loop_start_label = old_start;
    c.loop_end_label = old_end;
  }

};

struct ReturnNode : public ASTNode {
  expr_ptr_t value{nullptr};

  void assemble(CompilationContext& c) const override {
    if (value) {
      value->assemble(c, 3);
    }
    if (c.local_count + c.extra_offset) {
      c.code.push_back("set R0 " + std::to_string((c.local_count + c.extra_offset) * 4));
      c.code.emplace_back("add RS R0");
    }
    if (value) {
      c.code.emplace_back("mov R0 R3");
    }
    c.code.emplace_back("ret");
  }
};

struct BreakNode : public ASTNode {
  void assemble(CompilationContext& c) const override {
    if (c.loop_end_label.empty()) {
      throw compile_error("BREAK outside loop");
    }
    c.code.push_back("jmp " + c.loop_end_label);
  }
};

struct ContinueNode : public ASTNode {
  void assemble(CompilationContext& c) const override {
    if (c.loop_start_label.empty()) {
      throw compile_error("CONTINUE outside loop");
    }
    c.code.push_back("jmp " + c.loop_start_label);
  }
};

struct AssignNode : public ASTNode {
  expr_ptr_t target{nullptr};
  expr_ptr_t value{nullptr};

  void assemble(CompilationContext& c) const override {
    target->get_address(c, 4);
    c.code.emplace_back("push R4");
    ++c.extra_offset;
    value->assemble(c, 4);
    c.code.emplace_back("pop R0");
    --c.extra_offset;
    c.code.emplace_back("store32 R0 R4");
  }
};

struct ExecNode : public ASTNode {
  expr_ptr_t expr{nullptr};

  void assemble(CompilationContext& c) const override {
    expr->assemble(c, 0);
  }
};


struct Function {

  std::string name;
  std::vector<std::string> params;
  std::set<std::string> locals;
  std::set<std::pair<std::string, size_t>> called;
  std::vector<node_ptr_t> body;

  void assemble(CompilationContext& c) const {
    c.local_count = locals.size();
    c.extra_offset = 0;
    c.loop_start_label = "";
    c.loop_end_label = "";
    c.offsets.clear();
    size_t num = 0;
    for (const auto& p : params) {
      c.offsets[p] = locals.size() + params.size() - num++;
    }
    num = 0;
    for (const auto& l : locals) {
      c.offsets[l] = num++;
    }
    c.code.emplace_back("");
    c.code.push_back("@func_" + name + "_" + std::to_string(params.size()));
    if (!locals.empty()) {
      c.code.push_back("set R5 " + std::to_string(4 * locals.size()));
      c.code.emplace_back("sub RS R5");
    }
    for (const auto& op : body) {
      op->assemble(c);
    }
    if (!locals.empty()) {
      c.code.push_back("set R5 " + std::to_string(4 * locals.size()));
      c.code.emplace_back("add RS R5");
    }
    c.code.emplace_back("ret");
  }

};
