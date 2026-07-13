#ifndef OPCODE_TABLE_HPP
#define OPCODE_TABLE_HPP

#include "instruction.hpp"
#include <array>

namespace Emulator {

constexpr std::array<Instruction, 256> initializeOpcodeTable() {
    std::array<Instruction, 256> table{};

    // table.fill() isn't allowed here, so iteration is used instead
    for (auto& instruction : table) {
        instruction = {
            InstructionType::Unknown,
            AddressingMode::Unknown,
            0
        };
    }

    // Manually override each implemented opcode
    table[0xA9] = { InstructionType::LDA, AddressingMode::Immediate, 2 };

    return table;
}

};

#endif
