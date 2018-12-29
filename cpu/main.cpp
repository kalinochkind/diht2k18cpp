#include <iostream>
#include <fstream>
#include "cpu.h"

uint32_t input_func() {
  return static_cast<uint32_t>(std::cin.get());
}

void output_func(uint32_t c) {
  std::cout << static_cast<unsigned char>(c);
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
  std::vector<uint8_t> program((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());
  CPU cpu(640 * 1024);
  cpu.set_input_function(input_func);
  cpu.set_output_function(output_func);
  cpu.install_program(program);
  cpu.run_until_complete();
  return 0;
}