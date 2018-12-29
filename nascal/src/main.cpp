#include <iostream>
#include <fstream>
#include "parser.h"


int main(int argc, char** argv) {
  if (argc <= 1) {
    std::cerr << "Filename required" << std::endl;
    return 1;
  }
  std::ifstream infile(argv[1]);
  if (!infile.is_open()) {
    std::cerr << "Error: no such file" << std::endl;
    return 1;
  }
  std::string program((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());
  Tokenizer tokenizer(program);
  std::vector<Function> functions;
  try {
    functions = parse_program(tokenizer);
  } catch (const parse_error& e) {
    std::cerr << "PARSE ERROR\n" << e.what() << '\n' <<
              "Line " << tokenizer.get_line() << ", position " << tokenizer.get_linepos() << std::endl;
    return 1;
  }
  std::set<std::pair<std::string, size_t>> func_names = {{"printchar", 1},
                                                         {"readchar",  0}};
  for (const auto& f : functions) {
    func_names.insert({f.name, f.params.size()});
  }
  for (const auto& f : functions) {
    for (const auto& name : f.called) {
      if (!func_names.count(name)) {
        std::cerr << "COMPILE ERROR\n" << "Function " << f.name <<
                  " calls function " << name.first << " with " << name.second <<
                  (name.second == 1 ? " argument" : " arguments")
                  << " which is not defined" << std::endl;
        return 1;
      }
    }
  }

  if (!func_names.count({"main", 0})) {
    std::cerr << "COMPILE ERROR\nFunction main with no arguments does not exist" << std::endl;
    return 0;
  }

  CompilationContext context{};
  context.code.emplace_back("call @func_main_0");
  context.code.emplace_back("jmp @end");
  context.code.emplace_back("@func_printchar_1");
  context.code.emplace_back("set R0 4");
  context.code.emplace_back("add R0 RS");
  context.code.emplace_back("load32 R0 R0");
  context.code.emplace_back("out R0");
  context.code.emplace_back("ret");
  context.code.emplace_back("@func_readchar_0");
  context.code.emplace_back("in R0");
  context.code.emplace_back("ret");
  for (const auto& f : functions) {
    try {
      f.assemble(context);
    } catch (const compile_error& e) {
      std::cerr << "COMPILE ERROR\nFunction " << f.name << ": " << e.what() << std::endl;
      return 1;
    }
  }
  context.code.emplace_back("@end");

  for (const auto& s : context.code) {
    std::cout << s << '\n';
  }
  return 0;
}