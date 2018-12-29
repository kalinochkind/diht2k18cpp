#include <iostream>
#include <fstream>
#include <map>
#include <set>
#include "cpu.h"


 class DisassembleError: public std::runtime_error {
  using std::runtime_error::runtime_error;
};


std::string read_register(const std::vector<uint8_t> &program, size_t &pos) {
  if (pos >= program.size()) {
    throw DisassembleError("Unexpected end of file");
  }
  uint8_t reg = program[pos++];
  if (reg == REG_STACK) {
    return " RS";
  }
  if (reg == REG_INSTRUCTION) {
    return " RI";
  }
  return " R" + std::to_string(reg);
}

uint32_t get_value(const std::vector<uint8_t> &program, size_t &pos) {
  if (pos + 4 > program.size()) {
    throw DisassembleError("Unexpected end of file");
  }
  uint32_t value = static_cast<uint32_t>(program[pos]) | (static_cast<uint32_t>(program[pos + 1]) << 8) |
                   (static_cast<uint32_t>(program[pos + 2]) << 16) | (static_cast<uint32_t>(program[pos + 3]) << 24);
  pos += 4;
  return value;
}

std::string read_value(const std::vector<uint8_t> &program, size_t &pos) {
  return " " + std::to_string(get_value(program, pos));
}


std::string disassemble(const std::vector<uint8_t> &program) {
  size_t pos = 0;
  std::map<uint32_t, std::string> output{};
  std::set<uint32_t> labels;
  std::map<uint32_t, uint32_t> labels_wanted;
  while(pos < program.size()) {
    uint8_t command = program[pos++];
    if (command >= CPU::commands.size()) {
      throw DisassembleError("Invalid command at offset " + std::to_string(pos - 1));
    }
    std::string &line = output[pos - 1];
    line += CPU::commands[command].mnemonic;
    uint32_t label_pos = 0;
    switch(CPU::commands[command].type) {
      case command_type::REGREG:
        line += read_register(program, pos);
      case command_type::REG:
        line += read_register(program, pos);
        break;
      case command_type::REGVAL:
        line += read_register(program, pos);
        line += read_value(program, pos);
        break;
      case command_type::LABEL:
        label_pos = get_value(program, pos);
        labels.insert(label_pos);
        labels_wanted[pos - 5] = label_pos;
      case command_type::SIMPLE:
        ;
    }
  }
  std::string out_string;
  bool end_label_needed = false;
  for(const auto &p : output) {
    if (labels.count(p.first)) {
      out_string += "@l" + std::to_string(p.first) + "\n";
    }
    out_string += p.second;
    if (labels_wanted.count(p.first)) {
      if (output.count(labels_wanted[p.first])) {
        out_string += " @l" + std::to_string(labels_wanted[p.first]);
      } else if(labels_wanted[p.first] == pos) {
        out_string += " @end";
        end_label_needed = true;
      } else {
        out_string += " " + std::to_string(labels_wanted[p.first]);
      }
    }
    out_string += '\n';
  }
  if (end_label_needed) {
    out_string += "@end\n";
  }
  return out_string;
}



int main(int argc, char** argv) {
  if (argc <= 1) {
    std::cerr << "Filename required" << std::endl;
    return 1;
  }
  std::ifstream infile(argv[1], std::ifstream::binary | std::ifstream::in);
  if (!infile.is_open()) {
    std::cerr << "Error: no such file" << std::endl;
    return 1;
  }
  std::vector<uint8_t> program((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());
  std::string assembly = disassemble(program);
  std::cout << assembly;
}
