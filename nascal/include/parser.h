#pragma once
#include "tokenizer.h"
#include "ast.h"

expr_ptr_t parse_expr(Tokenizer& tokenizer, Function& function);


expr_ptr_t parse_expratom(Tokenizer& tokenizer, Function& function) {
  if (tokenizer.peek_token().type == token_type_t::UNARY_OP) {
    auto un = std::make_unique<UnaryExpression>();
    un->op = tokenizer.get_token().value[0];
    un->operand = parse_expratom(tokenizer, function);
    return un;
  }
  if (tokenizer.peek_token().type == token_type_t::INT) {
    auto num = std::make_unique<IntExpression>();
    num->value = std::stoi(tokenizer.get_token().value);
    return num;
  }
  if (tokenizer.peek_token().type == token_type_t::LEFT_PAR) {
    tokenizer.get_token();
    auto expr = parse_expr(tokenizer, function);
    if (tokenizer.get_token().type != token_type_t::RIGHT_PAR) {
      throw parse_error(") expected");
    }
    return expr;
  }
  if (tokenizer.peek_token().type != token_type_t::NAME) {
    throw parse_error("Expression expected");
  }
  auto name = tokenizer.get_token().value;
  if (tokenizer.peek_token().type != token_type_t::LEFT_PAR) {
    function.locals.insert(name);
    auto ne = std::make_unique<NameExpression>();
    ne->name = name;
    return ne;
  }
  tokenizer.get_token();
  auto func = std::make_unique<CallExpression>();
  func->func = name;
  while (tokenizer.peek_token().type != token_type_t::RIGHT_PAR) {
    auto expr = parse_expr(tokenizer, function);
    func->args.push_back(std::move(expr));
    if (tokenizer.peek_token().type == token_type_t::COMMA) {
      tokenizer.get_token();
    } else if (tokenizer.peek_token().type != token_type_t::RIGHT_PAR) {
      throw parse_error("Expression or ) expected");
    }
  }
  function.called.insert({name, func->args.size()});
  tokenizer.get_token();
  return func;
}

expr_ptr_t parse_exprmul(Tokenizer& tokenizer, Function& function) {
  expr_ptr_t expr = parse_expratom(tokenizer, function);
  while (tokenizer.peek_token().type == token_type_t::MUL_OP) {
    auto mul = std::make_unique<BinExpression>();
    mul->left = std::move(expr);
    mul->op = tokenizer.get_token().value[0];
    mul->right = parse_expratom(tokenizer, function);
    expr = std::move(mul);
  }
  return expr;
}

expr_ptr_t parse_expradd(Tokenizer& tokenizer, Function& function) {
  expr_ptr_t expr = parse_exprmul(tokenizer, function);
  while (tokenizer.peek_token().type == token_type_t::ADD_OP) {
    auto add = std::make_unique<BinExpression>();
    add->left = std::move(expr);
    add->op = tokenizer.get_token().value[0];
    add->right = parse_exprmul(tokenizer, function);
    expr = std::move(add);
  }
  return expr;
}

expr_ptr_t parse_expr(Tokenizer& tokenizer, Function& function) {
  expr_ptr_t expr = parse_expradd(tokenizer, function);
  if (tokenizer.peek_token().type != token_type_t::CMP_OP) {
    return expr;
  }
  auto cmp = std::make_unique<BinExpression>();
  cmp->left = std::move(expr);
  cmp->op = tokenizer.get_token().value[0];
  cmp->right = parse_expradd(tokenizer, function);
  return cmp;
}

node_ptr_t parse_op(Tokenizer& tokenizer, Function& function) {

  if (tokenizer.peek_token().type == token_type_t::WHILE) {
    auto node = std::make_unique<WhileNode>();
    tokenizer.get_token();
    node->condition = parse_expr(tokenizer, function);
    if (tokenizer.get_token().type != token_type_t::DO) {
      throw parse_error("DO expected");
    }
    while (tokenizer.peek_token().type != token_type_t::END) {
      if (tokenizer.peek_token().type == token_type_t::NONE) {
        throw parse_error("Unexpected end of while");
      }
      node->body.push_back(parse_op(tokenizer, function));
    }
    tokenizer.get_token();
    if (tokenizer.get_token().type != token_type_t::SEMICOLON) {
      throw parse_error("Missing ;");
    }
    return node;
  }

  if (tokenizer.peek_token().type == token_type_t::FOR) {
    auto node = std::make_unique<ForNode>();
    tokenizer.get_token();
    if (tokenizer.peek_token().type != token_type_t::NAME) {
      throw parse_error("Name expected");
    }
    node->var = parse_expr(tokenizer, function);
    if (tokenizer.get_token().type != token_type_t::ASSIGN) {
      throw parse_error(":= expected");
    }
    auto expr1 = parse_expr(tokenizer, function);
    if (tokenizer.peek_token().type == token_type_t::COMMA) {
      tokenizer.get_token();
      node->num_start = std::move(expr1);
      node->num_end = parse_expr(tokenizer, function);
      if (tokenizer.peek_token().type == token_type_t::COMMA) {
        tokenizer.get_token();
        node->num_step = parse_expr(tokenizer, function);
      }
    } else {
      node->num_end = std::move(expr1);
    }
    if (!node->num_start) {
      auto zero = std::make_unique<IntExpression>();
      zero->value = 0;
      node->num_start = std::move(zero);
    }
    if (!node->num_step) {
      auto one = std::make_unique<IntExpression>();
      one->value = 1;
      node->num_step = std::move(one);
    }
    if (tokenizer.get_token().type != token_type_t::DO) {
      throw parse_error("DO expected");
    }
    while (tokenizer.peek_token().type != token_type_t::END) {
      if (tokenizer.peek_token().type == token_type_t::NONE) {
        throw parse_error("Unexpected end of for");
      }
      node->body.push_back(parse_op(tokenizer, function));
    }
    tokenizer.get_token();
    if (tokenizer.get_token().type != token_type_t::SEMICOLON) {
      throw parse_error("Missing ;");
    }
    return node;
  }

  if (tokenizer.peek_token().type == token_type_t::IF) {
    auto node = std::make_unique<IfNode>();
    tokenizer.get_token();
    node->condition = parse_expr(tokenizer, function);
    if (tokenizer.get_token().type != token_type_t::THEN) {
      throw parse_error("THEN expected");
    }
    while (tokenizer.peek_token().type != token_type_t::END &&
           tokenizer.peek_token().type != token_type_t::ELSE) {
      if (tokenizer.peek_token().type == token_type_t::NONE) {
        throw parse_error("Unexpected end of if");
      }
      node->body.push_back(parse_op(tokenizer, function));
    }
    if (tokenizer.get_token().type == token_type_t::ELSE) {
      while (tokenizer.peek_token().type != token_type_t::END) {
        if (tokenizer.peek_token().type == token_type_t::NONE) {
          throw parse_error("Unexpected end of else");
        }
        node->else_body.push_back(parse_op(tokenizer, function));
      }
      tokenizer.get_token();
    }
    if (tokenizer.get_token().type != token_type_t::SEMICOLON) {
      throw parse_error("Missing ;");
    }
    return node;
  }

  if (tokenizer.peek_token().type == token_type_t::RETURN) {
    auto node = std::make_unique<ReturnNode>();
    tokenizer.get_token();
    if (tokenizer.peek_token().type != token_type_t::SEMICOLON) {
      node->value = parse_expr(tokenizer, function);
    }
    if (tokenizer.get_token().type != token_type_t::SEMICOLON) {
      throw parse_error("; expected");
    }
    return node;
  }

  if (tokenizer.peek_token().type == token_type_t::BREAK) {
    tokenizer.get_token();
    auto node = std::make_unique<BreakNode>();
    if (tokenizer.get_token().type != token_type_t::SEMICOLON) {
      throw parse_error("; expected");
    }
    return node;
  }

  if (tokenizer.peek_token().type == token_type_t::CONTINUE) {
    auto node = std::make_unique<ContinueNode>();
    tokenizer.get_token();
    if (tokenizer.get_token().type != token_type_t::SEMICOLON) {
      throw parse_error("; expected");
    }
    return node;
  }

  auto expr = parse_expr(tokenizer, function);
  if (tokenizer.peek_token().type == token_type_t::ASSIGN) {
    tokenizer.get_token();
    auto node = std::make_unique<AssignNode>();
    node->target = std::move(expr);
    node->value = parse_expr(tokenizer, function);
    if (tokenizer.get_token().type != token_type_t::SEMICOLON) {
      throw parse_error("; expected");
    }
    return node;
  }
  if (tokenizer.get_token().type != token_type_t::SEMICOLON) {
    throw parse_error("; expected");
  }
  auto node = std::make_unique<ExecNode>();
  node->expr = std::move(expr);
  return node;
}


Function parse_def(Tokenizer& tokenizer) {
  Function func;
  if (tokenizer.get_token().type != token_type_t::DEF) {
    throw parse_error("DEF expected");
  }
  Token name = tokenizer.get_token();
  if (name.type != token_type_t::NAME) {
    throw parse_error("Name expected");
  }
  func.name = name.value;
  if (tokenizer.get_token().type != token_type_t::LEFT_PAR) {
    throw parse_error("( expected");
  }
  while (tokenizer.peek_token().type == token_type_t::NAME) {
    func.params.push_back(tokenizer.get_token().value);
    if (tokenizer.peek_token().type == token_type_t::COMMA) {
      tokenizer.get_token();
    } else {
      break;
    }
  }
  if (tokenizer.get_token().type != token_type_t::RIGHT_PAR) {
    throw parse_error("Name or ) expected");
  }
  while (tokenizer.peek_token().type != token_type_t::END) {
    if (tokenizer.peek_token().type == token_type_t::NONE) {
      throw parse_error("Unexpected end of function");
    }
    func.body.push_back(parse_op(tokenizer, func));
  }
  tokenizer.get_token();
  if (tokenizer.get_token().type != token_type_t::SEMICOLON) {
    throw parse_error("Missing ;");
  }
  for (const auto& s : func.params) {
    func.locals.erase(s);
  }
  return func;
}

std::vector<Function> parse_program(Tokenizer& tokenizer) {
  Token t;
  std::vector<Function> res;
  while (tokenizer.peek_token().type != token_type_t::NONE) {
    res.push_back(parse_def(tokenizer));
  }
  return res;
}