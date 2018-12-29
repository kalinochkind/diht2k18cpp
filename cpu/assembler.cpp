#include <iostream>
#include <fstream>
#include <map>
#include "cpu.h"

class AssembleError : public std::runtime_error {
  using std::runtime_error::runtime_error;
};


class Tokenizer {

 public:
  Tokenizer(std::string s)
      : data(std::move(s)) {
    if (!data.empty() && data.back() != '\n') {
      data.push_back('\n');
    }
  };

  std::string get_token() {
    if (position >= data.size()) {
      return "";
    }
    skip_whitespace();
    return read_token();
  }

  size_t get_line_number() const {
    return line_number;
  }

 private:

  void skip_whitespace() {
    while (data[position] == ' ' || data[position] == '\t') {
      ++position;
    }
    if (data[position] == ';') {
      while (data[position] != '\n') {
        ++position;
      }
    }
  }

  std::string read_token() {
    std::string res;
    if (data[position] == '\n') {
      ++position;
      ++line_number;
      return "\n";
    }
    while (data[position] != ' ' && data[position] != '\t' && data[position] != '\n') {
      res.push_back(data[position++]);
    }
    return res;
  }

  std::string data;
  size_t position{0};
  size_t line_number{0};
};

void error(const std::string& error, size_t line_no) {
  std::cerr << "[Line " << line_no << "] " << error << std::endl;
  exit(1);
}

bool is_digit(const std::string& s) {
  for (char c : s) {
    if (!isdigit(c)) {
      return false;
    }
  }
  return true;
}


uint8_t get_command(const std::string& name, size_t line_number) {
  for (size_t i = 0; i < CPU::commands.size(); ++i) {
    if (CPU::commands[i].mnemonic == name) {
      return static_cast<uint8_t>(i);
    }
  }
  error("Invalid command: " + name, line_number);
  return 0;
}

uint8_t get_register(const std::string& name, size_t line_number) {
  if (name == "RI") {
    return REG_INSTRUCTION;
  }
  if (name == "RS") {
    return REG_STACK;
  }
  if (name.size() < 2 || name.length() > 4 || name[0] != 'R' || !is_digit(name.substr(1))) {
    error("Invalid register name: " + name, line_number);
  }
  auto num = std::stoul(name.substr(1));
  if (num >= 256) {
    error("Invalid register name: " + name, line_number);
  }
  return static_cast<uint8_t>(num);
}

void push_uint32(std::vector<uint8_t>& vec, uint32_t num) {
  vec.push_back(static_cast<uint8_t>(num));
  vec.push_back(static_cast<uint8_t>(num >> 8));
  vec.push_back(static_cast<uint8_t>(num >> 16));
  vec.push_back(static_cast<uint8_t>(num >> 24));
}

void push_long_value(std::vector<uint8_t>& vec, const std::string& value, std::map<std::string, uint32_t>& labels,
                     size_t line_number, bool second_run) {
  if (value[0] == '@') {
    if (labels.count(value)) {
      push_uint32(vec, labels[value]);
    } else if (second_run) {
      error("Label not declared: " + value, line_number);
    } else {
      push_uint32(vec, 0);
    }
  } else {
    long long val{0};
    try {
      val = std::stoll(value);
    } catch (const std::exception&) {
      error("Invalid number: " + value, line_number);
    }
    push_uint32(vec, static_cast<uint32_t>(val));
  }
}

std::vector<uint8_t> assemble(std::string program, std::map<std::string, uint32_t>& labels, bool second_run) {
  Tokenizer tokenizer(std::move(program));
  std::vector<uint8_t> output{};
  std::vector<std::string> line{};
  std::string token{};
  while (true) {
    line.clear();
    do {
      line.push_back(tokenizer.get_token());
    } while (!line.back().empty() && line.back() != "\n");
    if (line.front().empty()) {
      break;
    }
    if (line.size() == 1) {
      continue;
    }
    if (line[0][0] == '@') {
      if (line.size() > 2) {
        error("Invalid label declaration", tokenizer.get_line_number());
      }
      if (labels.count(line[0]) && labels[line[0]] != static_cast<uint32_t>(output.size())) {
        error("Label redeclared: " + line[0], tokenizer.get_line_number());
      }
      labels[line[0]] = static_cast<uint32_t>(output.size());
      continue;
    }
    uint8_t command_index = get_command(line[0], tokenizer.get_line_number());
    output.push_back(command_index);
    switch (CPU::commands[command_index].type) {
      case command_type::SIMPLE:
        if (line.size() != 2) {
          error("Syntax error", tokenizer.get_line_number());
        }
        break;
      case command_type::REG:
        if (line.size() != 3) {
          error("Syntax error", tokenizer.get_line_number());
        }
        output.push_back(get_register(line[1], tokenizer.get_line_number()));
        break;
      case command_type::REGREG:
        if (line.size() != 4) {
          error("Syntax error", tokenizer.get_line_number());
        }
        output.push_back(get_register(line[1], tokenizer.get_line_number()));
        output.push_back(get_register(line[2], tokenizer.get_line_number()));
        break;
      case command_type::LABEL:
        if (line.size() != 3) {
          error("Syntax error", tokenizer.get_line_number());
        }
        push_long_value(output, line[1], labels, tokenizer.get_line_number(), second_run);
        break;
      case command_type::REGVAL:
        if (line.size() != 4) {
          throw AssembleError("Invalid instruction");
        }
        output.push_back(get_register(line[1], tokenizer.get_line_number()));
        push_long_value(output, line[2], labels, tokenizer.get_line_number(), second_run);
        break;
    }
  }
  return output;
}

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
  std::map<std::string, uint32_t> labels;
  assemble(program, labels, false);
  auto res = assemble(program, labels, true);
  for (uint8_t c : res) {
    std::cout << static_cast<unsigned char>(c);
  }
}
