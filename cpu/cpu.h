#pragma once

#include <cstdint>
#include <stdexcept>
#include <vector>
#include <functional>

constexpr uint8_t REG_STACK = 0xfe;
constexpr uint8_t REG_INSTRUCTION = 0xff;

constexpr uint8_t CPU_VERSION = 1;

enum class command_type {
  SIMPLE, REG, REGREG, REGVAL, LABEL
};

class CPU;

struct CommandData {

  uint8_t reg1{0};
  uint8_t reg2{0};
  uint32_t value{0};
};

using command_handler = std::function<void(CPU&, const CommandData&)>;

struct Command {

  Command(std::string mnemonic, command_type type, bool sets_flags, command_handler handler)
      : handler(std::move(handler)), mnemonic(std::move(mnemonic)), type(type), sets_flags(sets_flags) {
  };

  command_handler handler;
  std::string mnemonic;
  command_type type;
  bool sets_flags;


};

class CPUError : public std::runtime_error {
  using std::runtime_error::runtime_error;
};


struct Flags {
  bool zero{false};
  bool sign{false};
  bool overflow{false};

  void clear() {
    zero = false;
    sign = false;
    overflow = false;
  }

  void set_from(uint32_t value) {
    zero = !value;
    sign = static_cast<int32_t>(value) < 0;
  }
};


class CPU {

 public:
  CPU(uint32_t memory_size)
      : memory(memory_size) {
  };

  void install_program(const std::vector<uint8_t>& program) {
    if (program.size() > memory.size()) {
      throw CPUError("Not enough memory");
    }
    std::fill(memory.begin(), memory.end() - program.size(), 0);
    std::copy(program.begin(), program.end(), memory.end() - program.size());
    std::fill(registers.begin(), registers.end(), 0);
    registers[REG_INSTRUCTION] = static_cast<uint32_t>(memory.size() - program.size());
    program_offset = registers[REG_INSTRUCTION];
    registers[REG_STACK] = program_offset;
    flags.clear();
  }

  bool run_command() {
    uint32_t addr = registers[REG_INSTRUCTION];
    if (addr >= memory.size()) {
      return true;
    }
    uint8_t command_id = memory[addr];
    if (command_id >= commands.size()) {
      throw CPUError("Invalid command");
    }
    shifted_ri = addr + 1;

    CommandData data{};
    switch (commands[command_id].type) {
      case command_type::REG:
        data.reg1 = get_value_8();
        break;
      case command_type::REGREG:
        data.reg1 = get_value_8();
        data.reg2 = get_value_8();
        break;
      case command_type::REGVAL:
        data.reg1 = get_value_8();
      case command_type::LABEL:
        data.value = get_value_32();
      case command_type::SIMPLE:;
    }

    commands[command_id].handler(*this, data);
    if (commands[command_id].sets_flags) {
      flags.set_from(registers[data.reg1]);
    }
    registers[REG_INSTRUCTION] = shifted_ri;

    return false;
  }

  void run_until_complete() {
    while (!run_command()) {
    };
  }

  void set_input_function(std::function<uint32_t(void)> f) {
    input_function = std::move(f);
  }

  void set_output_function(std::function<void(uint32_t)> f) {
    output_function = std::move(f);
  }

  static const std::vector<Command> commands;

 private:

  uint8_t get_value_8() {
    ++shifted_ri;
    if (shifted_ri > memory.size()) {
      throw CPUError("Truncated command");
    }
    return memory[shifted_ri - 1];
  }

  uint32_t get_value_32() {
    shifted_ri += 4;
    if (shifted_ri > memory.size()) {
      throw CPUError("Truncated command");
    }
    return read_from_memory_32(shifted_ri - 4);
  }

  void write_to_memory_8(uint32_t addr, uint8_t value) {
    if (static_cast<size_t>(addr) + 1 > memory.size()) {
      throw CPUError("Invalid write");
    }
    memory[addr] = value;
  }

  void write_to_memory_16(uint32_t addr, uint16_t value) {
    if (static_cast<size_t>(addr) + 2 > memory.size()) {
      throw CPUError("Invalid write");
    }
    memory[addr] = static_cast<uint8_t>(value);
    memory[addr + 1] = static_cast<uint8_t>(value >> 8);
  }

  void write_to_memory_32(uint32_t addr, uint32_t value) {
    if (static_cast<size_t>(addr) + 4 > memory.size()) {
      throw CPUError("Invalid write");
    }
    memory[addr] = static_cast<uint8_t>(value);
    memory[addr + 1] = static_cast<uint8_t>(value >> 8);
    memory[addr + 2] = static_cast<uint8_t>(value >> 16);
    memory[addr + 3] = static_cast<uint8_t>(value >> 24);
  }

  uint8_t read_from_memory_8(uint32_t addr) {
    if (static_cast<size_t>(addr) + 1 > memory.size()) {
      throw CPUError("Invalid read");
    }
    return memory[addr];
  }

  uint16_t read_from_memory_16(uint32_t addr) {
    if (static_cast<size_t>(addr) + 2 > memory.size()) {
      throw CPUError("Invalid read");
    }
    return static_cast<uint16_t>(memory[addr]) | (static_cast<uint16_t>(memory[addr + 1]) << 8);
  }

  uint32_t read_from_memory_32(uint32_t addr) {
    if (static_cast<size_t>(addr) + 4 > memory.size()) {
      throw CPUError("Invalid read");
    }
    return static_cast<uint32_t>(memory[addr]) | (static_cast<uint32_t>(memory[addr + 1]) << 8) |
           (static_cast<uint32_t>(memory[addr + 2]) << 16) | (static_cast<uint32_t>(memory[addr + 3]) << 24);
  }

  void push_on_stack(uint32_t value) {
    write_to_memory_32(registers[REG_STACK] -= 4, value);
  }

  uint32_t pop_from_stack() {
    uint32_t value = read_from_memory_32(registers[REG_STACK]);
    registers[REG_STACK] += 4;
    return value;
  }

  void check_division_argument(uint32_t value) {
    if (value == 0) {
      throw CPUError("Division by zero");
    }
  }

 private:
  std::vector<uint8_t> memory;
  std::array<uint32_t, 256> registers;
  uint32_t program_offset{0};
  uint32_t shifted_ri{0};
  std::function<uint32_t(void)> input_function{nullptr};
  std::function<void(uint32_t)> output_function{nullptr};
  Flags flags;
};


const std::vector<Command> CPU::commands =
    {
        {"nop",     command_type::SIMPLE, false, [](CPU& cpu, const CommandData& data) {
        }},
        {"stat",    command_type::SIMPLE, false, [](CPU& cpu, const CommandData& data) {
          cpu.registers[0] = CPU_VERSION;
          cpu.registers[1] = static_cast<uint32_t>(cpu.memory.size());
          cpu.registers[2] = cpu.program_offset;
        }},
        {"set",     command_type::REGVAL, false, [](CPU& cpu, const CommandData& data) {
          cpu.registers[data.reg1] = data.value;
        }},
        {"in",      command_type::REG,    false, [](CPU& cpu, const CommandData& data) {
          cpu.registers[data.reg1] = cpu.input_function();
        }},
        {"out",     command_type::REG,    false, [](CPU& cpu, const CommandData& data) {
          cpu.output_function(cpu.registers[data.reg1]);
        }},
        {"store8",  command_type::REGREG, false, [](CPU& cpu, const CommandData& data) {
          cpu.write_to_memory_8(cpu.registers[data.reg1], static_cast<uint8_t>(cpu.registers[data.reg2]));
        }},
        {"store16", command_type::REGREG, false, [](CPU& cpu, const CommandData& data) {
          cpu.write_to_memory_16(cpu.registers[data.reg1], static_cast<uint16_t>(cpu.registers[data.reg2]));
        }},
        {"store32", command_type::REGREG, false, [](CPU& cpu, const CommandData& data) {
          cpu.write_to_memory_32(cpu.registers[data.reg1], cpu.registers[data.reg2]);
        }},
        {"load8",   command_type::REGREG, false, [](CPU& cpu, const CommandData& data) {
          cpu.registers[data.reg1] = cpu.read_from_memory_8(cpu.registers[data.reg2]);
        }},
        {"load16",  command_type::REGREG, false, [](CPU& cpu, const CommandData& data) {
          cpu.registers[data.reg1] = cpu.read_from_memory_16(cpu.registers[data.reg2]);
        }},
        {"load32",  command_type::REGREG, false, [](CPU& cpu, const CommandData& data) {
          cpu.registers[data.reg1] = cpu.read_from_memory_32(cpu.registers[data.reg2]);
        }},
        {"push",    command_type::REG,    false, [](CPU& cpu, const CommandData& data) {
          cpu.push_on_stack(cpu.registers[data.reg1]);
        }},
        {"pop",     command_type::REG,    false, [](CPU& cpu, const CommandData& data) {
          cpu.registers[data.reg1] = cpu.pop_from_stack();
        }},
        {"mov",     command_type::REGREG, false, [](CPU& cpu, const CommandData& data) {
          cpu.registers[data.reg1] = cpu.registers[data.reg2];
        }},
        {"add",     command_type::REGREG, true,  [](CPU& cpu, const CommandData& data) {
          cpu.registers[data.reg1] += cpu.registers[data.reg2];
          cpu.flags.overflow = cpu.registers[data.reg1] < cpu.registers[data.reg2];
        }},
        {"sub",     command_type::REGREG, true,  [](CPU& cpu, const CommandData& data) {
          cpu.flags.overflow = cpu.registers[data.reg1] < cpu.registers[data.reg2];
          cpu.registers[data.reg1] -= cpu.registers[data.reg2];
        }},
        {"smul",    command_type::REGREG, true,  [](CPU& cpu, const CommandData& data) {
          cpu.registers[data.reg1] = static_cast<uint32_t>(static_cast<int32_t>(cpu.registers[data.reg1]) *
                                                           static_cast<int32_t>(cpu.registers[data.reg2]));
        }},
        {"umul",    command_type::REGREG, true,  [](CPU& cpu, const CommandData& data) {
          cpu.registers[data.reg1] *= cpu.registers[data.reg2];
        }},
        {"sdiv",    command_type::REGREG, true,  [](CPU& cpu, const CommandData& data) {
          cpu.check_division_argument(cpu.registers[data.reg2]);
          cpu.registers[data.reg1] = static_cast<uint32_t>(static_cast<int32_t>(cpu.registers[data.reg1]) /
                                                           static_cast<int32_t>(cpu.registers[data.reg2]));
        }},
        {"udiv",    command_type::REGREG, true,  [](CPU& cpu, const CommandData& data) {
          cpu.check_division_argument(cpu.registers[data.reg2]);
          cpu.registers[data.reg1] /= cpu.registers[data.reg2];
        }},
        {"smod",    command_type::REGREG, true,  [](CPU& cpu, const CommandData& data) {
          cpu.check_division_argument(cpu.registers[data.reg2]);
          cpu.registers[data.reg1] = static_cast<uint32_t>(static_cast<int32_t>(cpu.registers[data.reg1]) %
                                                           static_cast<int32_t>(cpu.registers[data.reg2]));
        }},
        {"umod",    command_type::REGREG, true,  [](CPU& cpu, const CommandData& data) {
          cpu.check_division_argument(cpu.registers[data.reg2]);
          cpu.registers[data.reg1] %= cpu.registers[data.reg2];
        }},
        {"neg",     command_type::REG,    true,  [](CPU& cpu, const CommandData& data) {
          cpu.registers[data.reg1] = static_cast<uint32_t>(-static_cast<int32_t>(cpu.registers[data.reg1]));
        }},
        {"and",     command_type::REGREG, true,  [](CPU& cpu, const CommandData& data) {
          cpu.registers[data.reg1] &= cpu.registers[data.reg2];
        }},
        {"or",      command_type::REGREG, true,  [](CPU& cpu, const CommandData& data) {
          cpu.registers[data.reg1] |= cpu.registers[data.reg2];
        }},
        {"xor",     command_type::REGREG, true,  [](CPU& cpu, const CommandData& data) {
          cpu.registers[data.reg1] ^= cpu.registers[data.reg2];
        }},
        {"shift",   command_type::REGREG, true,  [](CPU& cpu, const CommandData& data) {
          if (static_cast<int32_t>(cpu.registers[data.reg2]) >= 0) {
            cpu.registers[data.reg1] <<= static_cast<int32_t>(cpu.registers[data.reg2]);
          } else {
            cpu.registers[data.reg1] >>= -static_cast<int32_t>(cpu.registers[data.reg2]);
          }
        }},
        {"not",     command_type::REG,    true,  [](CPU& cpu, const CommandData& data) {
          cpu.registers[data.reg1] = ~cpu.registers[data.reg1];
        }},
        {"call",    command_type::LABEL,    false, [](CPU& cpu, const CommandData& data) {
          cpu.push_on_stack(cpu.shifted_ri);
          cpu.shifted_ri = data.value + cpu.program_offset;
        }},
        {"ret",     command_type::SIMPLE, false, [](CPU& cpu, const CommandData& data) {
          cpu.shifted_ri = cpu.pop_from_stack();
        }},
        {"jmp",     command_type::LABEL,    false, [](CPU& cpu, const CommandData& data) {
          cpu.shifted_ri = data.value + cpu.program_offset;
        }},
        {"jiz",     command_type::LABEL,    false, [](CPU& cpu, const CommandData& data) {
          if (cpu.flags.zero) {
            cpu.shifted_ri = data.value + cpu.program_offset;
          }
        }},
        {"juz",     command_type::LABEL,    false, [](CPU& cpu, const CommandData& data) {
          if (!cpu.flags.zero) {
            cpu.shifted_ri = data.value + cpu.program_offset;
          }
        }},
        {"jis",     command_type::LABEL,    false, [](CPU& cpu, const CommandData& data) {
          if (cpu.flags.sign) {
            cpu.shifted_ri = data.value + cpu.program_offset;
          }
        }},
        {"jus",     command_type::LABEL,    false, [](CPU& cpu, const CommandData& data) {
          if (!cpu.flags.sign) {
            cpu.shifted_ri = data.value + cpu.program_offset;
          }
        }},
        {"jio",     command_type::LABEL,    false, [](CPU& cpu, const CommandData& data) {
          if (cpu.flags.overflow) {
            cpu.shifted_ri = data.value + cpu.program_offset;
          }
        }},
        {"juo",     command_type::LABEL,    false, [](CPU& cpu, const CommandData& data) {
          if (!cpu.flags.overflow) {
            cpu.shifted_ri = data.value + cpu.program_offset;
          }
        }},
        {"jmpr",    command_type::REG,    false, [](CPU& cpu, const CommandData& data) {
          cpu.shifted_ri = cpu.registers[data.reg1] + cpu.program_offset;
        }},
    };