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

    // ADC
    table[0x69] = { InstructionType::ADC, AddressingMode::Immediate, 2 };
    table[0x65] = { InstructionType::ADC, AddressingMode::ZeroPage,  3 };
    table[0x75] = { InstructionType::ADC, AddressingMode::ZeroPageX, 4 };
    table[0x6D] = { InstructionType::ADC, AddressingMode::Absolute,  4 };
    table[0x7D] = { InstructionType::ADC, AddressingMode::AbsoluteX, 4 };
    table[0x79] = { InstructionType::ADC, AddressingMode::AbsoluteY, 4 };
    table[0x61] = { InstructionType::ADC, AddressingMode::IndirectX, 6 };
    table[0x71] = { InstructionType::ADC, AddressingMode::IndirectY, 5 };

    // AND
    table[0x29] = { InstructionType::AND, AddressingMode::Immediate, 2 };
    table[0x25] = { InstructionType::AND, AddressingMode::ZeroPage,  3 };
    table[0x35] = { InstructionType::AND, AddressingMode::ZeroPageX, 4 };
    table[0x2D] = { InstructionType::AND, AddressingMode::Absolute,  4 };
    table[0x3D] = { InstructionType::AND, AddressingMode::AbsoluteX, 4 };
    table[0x39] = { InstructionType::AND, AddressingMode::AbsoluteY, 4 };
    table[0x21] = { InstructionType::AND, AddressingMode::IndirectX, 6 };
    table[0x31] = { InstructionType::AND, AddressingMode::IndirectY, 5 };

    // ASL
    table[0x0A] = { InstructionType::ASL, AddressingMode::A,         2 };
    table[0x06] = { InstructionType::ASL, AddressingMode::ZeroPage,  5 };
    table[0x16] = { InstructionType::ASL, AddressingMode::ZeroPageX, 6 };
    table[0x0E] = { InstructionType::ASL, AddressingMode::Absolute,  6 };
    table[0x1E] = { InstructionType::ASL, AddressingMode::AbsoluteX, 7 };

    // Branches
    table[0x90] = { InstructionType::BCC, AddressingMode::Relative, 2 };
    table[0xB0] = { InstructionType::BCS, AddressingMode::Relative, 2 };
    table[0xF0] = { InstructionType::BEQ, AddressingMode::Relative, 2 };
    table[0x30] = { InstructionType::BMI, AddressingMode::Relative, 2 };
    table[0xD0] = { InstructionType::BNE, AddressingMode::Relative, 2 };
    table[0x10] = { InstructionType::BPL, AddressingMode::Relative, 2 };
    table[0x50] = { InstructionType::BVC, AddressingMode::Relative, 2 };
    table[0x70] = { InstructionType::BVS, AddressingMode::Relative, 2 };

    // BIT
    table[0x24] = { InstructionType::BIT, AddressingMode::ZeroPage, 3 };
    table[0x2C] = { InstructionType::BIT, AddressingMode::Absolute, 4 };

    // BRK
    table[0x00] = { InstructionType::BRK, AddressingMode::Implied, 7 };

    // Clear flags
    table[0x18] = { InstructionType::CLC, AddressingMode::Implied, 2 };
    table[0xD8] = { InstructionType::CLD, AddressingMode::Implied, 2 };
    table[0x58] = { InstructionType::CLI, AddressingMode::Implied, 2 };
    table[0xB8] = { InstructionType::CLV, AddressingMode::Implied, 2 };

    // CMP
    table[0xC9] = { InstructionType::CMP, AddressingMode::Immediate, 2 };
    table[0xC5] = { InstructionType::CMP, AddressingMode::ZeroPage,  3 };
    table[0xD5] = { InstructionType::CMP, AddressingMode::ZeroPageX, 4 };
    table[0xCD] = { InstructionType::CMP, AddressingMode::Absolute,  4 };
    table[0xDD] = { InstructionType::CMP, AddressingMode::AbsoluteX, 4 };
    table[0xD9] = { InstructionType::CMP, AddressingMode::AbsoluteY, 4 };
    table[0xC1] = { InstructionType::CMP, AddressingMode::IndirectX, 6 };
    table[0xD1] = { InstructionType::CMP, AddressingMode::IndirectY, 5 };

    // CPX
    table[0xE0] = { InstructionType::CPX, AddressingMode::Immediate, 2 };
    table[0xE4] = { InstructionType::CPX, AddressingMode::ZeroPage,  3 };
    table[0xEC] = { InstructionType::CPX, AddressingMode::Absolute,  4 };

    // CPY
    table[0xC0] = { InstructionType::CPY, AddressingMode::Immediate, 2 };
    table[0xC4] = { InstructionType::CPY, AddressingMode::ZeroPage,  3 };
    table[0xCC] = { InstructionType::CPY, AddressingMode::Absolute,  4 };

    // DEC
    table[0xC6] = { InstructionType::DEC, AddressingMode::ZeroPage,  5 };
    table[0xD6] = { InstructionType::DEC, AddressingMode::ZeroPageX, 6 };
    table[0xCE] = { InstructionType::DEC, AddressingMode::Absolute,  6 };
    table[0xDE] = { InstructionType::DEC, AddressingMode::AbsoluteX, 7 };

    // DEX / DEY
    table[0xCA] = { InstructionType::DEX, AddressingMode::Implied, 2 };
    table[0x88] = { InstructionType::DEY, AddressingMode::Implied, 2 };

    // EOR
    table[0x49] = { InstructionType::EOR, AddressingMode::Immediate, 2 };
    table[0x45] = { InstructionType::EOR, AddressingMode::ZeroPage,  3 };
    table[0x55] = { InstructionType::EOR, AddressingMode::ZeroPageX, 4 };
    table[0x4D] = { InstructionType::EOR, AddressingMode::Absolute,  4 };
    table[0x5D] = { InstructionType::EOR, AddressingMode::AbsoluteX, 4 };
    table[0x59] = { InstructionType::EOR, AddressingMode::AbsoluteY, 4 };
    table[0x41] = { InstructionType::EOR, AddressingMode::IndirectX, 6 };
    table[0x51] = { InstructionType::EOR, AddressingMode::IndirectY, 5 };

    // INC
    table[0xE6] = { InstructionType::INC, AddressingMode::ZeroPage,  5 };
    table[0xF6] = { InstructionType::INC, AddressingMode::ZeroPageX, 6 };
    table[0xEE] = { InstructionType::INC, AddressingMode::Absolute,  6 };
    table[0xFE] = { InstructionType::INC, AddressingMode::AbsoluteX, 7 };

    // INX / INY
    table[0xE8] = { InstructionType::INX, AddressingMode::Implied, 2 };
    table[0xC8] = { InstructionType::INY, AddressingMode::Implied, 2 };

    // JMP
    table[0x4C] = { InstructionType::JMP, AddressingMode::Absolute, 3 };
    table[0x6C] = { InstructionType::JMP, AddressingMode::Indirect, 5 };

    // JSR
    table[0x20] = { InstructionType::JSR, AddressingMode::Absolute, 6 };

    // LDA
    table[0xA9] = { InstructionType::LDA, AddressingMode::Immediate, 2 };
    table[0xA5] = { InstructionType::LDA, AddressingMode::ZeroPage,  3 };
    table[0xB5] = { InstructionType::LDA, AddressingMode::ZeroPageX, 4 };
    table[0xAD] = { InstructionType::LDA, AddressingMode::Absolute,  4 };
    table[0xBD] = { InstructionType::LDA, AddressingMode::AbsoluteX, 4 };
    table[0xB9] = { InstructionType::LDA, AddressingMode::AbsoluteY, 4 };
    table[0xA1] = { InstructionType::LDA, AddressingMode::IndirectX, 6 };
    table[0xB1] = { InstructionType::LDA, AddressingMode::IndirectY, 5 };

    // LDX
    table[0xA2] = { InstructionType::LDX, AddressingMode::Immediate, 2 };
    table[0xA6] = { InstructionType::LDX, AddressingMode::ZeroPage,  3 };
    table[0xB6] = { InstructionType::LDX, AddressingMode::ZeroPageY, 4 };
    table[0xAE] = { InstructionType::LDX, AddressingMode::Absolute,  4 };
    table[0xBE] = { InstructionType::LDX, AddressingMode::AbsoluteY, 4 };

    // LDY
    table[0xA0] = { InstructionType::LDY, AddressingMode::Immediate, 2 };
    table[0xA4] = { InstructionType::LDY, AddressingMode::ZeroPage,  3 };
    table[0xB4] = { InstructionType::LDY, AddressingMode::ZeroPageX, 4 };
    table[0xAC] = { InstructionType::LDY, AddressingMode::Absolute,  4 };
    table[0xBC] = { InstructionType::LDY, AddressingMode::AbsoluteX, 4 };

    // LSR
    table[0x4A] = { InstructionType::LSR, AddressingMode::A,         2 };
    table[0x46] = { InstructionType::LSR, AddressingMode::ZeroPage,  5 };
    table[0x56] = { InstructionType::LSR, AddressingMode::ZeroPageX, 6 };
    table[0x4E] = { InstructionType::LSR, AddressingMode::Absolute,  6 };
    table[0x5E] = { InstructionType::LSR, AddressingMode::AbsoluteX, 7 };

    // NOP
    table[0xEA] = { InstructionType::NOP, AddressingMode::Implied, 2 };

    // ORA
    table[0x09] = { InstructionType::ORA, AddressingMode::Immediate, 2 };
    table[0x05] = { InstructionType::ORA, AddressingMode::ZeroPage,  3 };
    table[0x15] = { InstructionType::ORA, AddressingMode::ZeroPageX, 4 };
    table[0x0D] = { InstructionType::ORA, AddressingMode::Absolute,  4 };
    table[0x1D] = { InstructionType::ORA, AddressingMode::AbsoluteX, 4 };
    table[0x19] = { InstructionType::ORA, AddressingMode::AbsoluteY, 4 };
    table[0x01] = { InstructionType::ORA, AddressingMode::IndirectX, 6 };
    table[0x11] = { InstructionType::ORA, AddressingMode::IndirectY, 5 };

    // Push / Pull Stack
    table[0x48] = { InstructionType::PHA, AddressingMode::Implied, 3 };
    table[0x08] = { InstructionType::PHP, AddressingMode::Implied, 3 };
    table[0x68] = { InstructionType::PLA, AddressingMode::Implied, 4 };
    table[0x28] = { InstructionType::PLP, AddressingMode::Implied, 4 };

    // ROL
    table[0x2A] = { InstructionType::ROL, AddressingMode::A,         2 };
    table[0x26] = { InstructionType::ROL, AddressingMode::ZeroPage,  5 };
    table[0x36] = { InstructionType::ROL, AddressingMode::ZeroPageX, 6 };
    table[0x2E] = { InstructionType::ROL, AddressingMode::Absolute,  6 };
    table[0x3E] = { InstructionType::ROL, AddressingMode::AbsoluteX, 7 };

    // Returns
    table[0x40] = { InstructionType::RTI, AddressingMode::Implied, 6 };
    table[0x60] = { InstructionType::RTS, AddressingMode::Implied, 6 };

    // SBC
    table[0xE9] = { InstructionType::SBC, AddressingMode::Immediate, 2 };
    table[0xE5] = { InstructionType::SBC, AddressingMode::ZeroPage,  3 };
    table[0xF5] = { InstructionType::SBC, AddressingMode::ZeroPageX, 4 };
    table[0xED] = { InstructionType::SBC, AddressingMode::Absolute,  4 };
    table[0xFD] = { InstructionType::SBC, AddressingMode::AbsoluteX, 4 };
    table[0xF9] = { InstructionType::SBC, AddressingMode::AbsoluteY, 4 };
    table[0xE1] = { InstructionType::SBC, AddressingMode::IndirectX, 6 };
    table[0xF1] = { InstructionType::SBC, AddressingMode::IndirectY, 5 };

    // Set flags
    table[0x38] = { InstructionType::SEC, AddressingMode::Implied, 2 };
    table[0xF8] = { InstructionType::SED, AddressingMode::Implied, 2 };
    table[0x78] = { InstructionType::SEI, AddressingMode::Implied, 2 };

    // STA
    table[0x85] = { InstructionType::STA, AddressingMode::ZeroPage,  3 };
    table[0x95] = { InstructionType::STA, AddressingMode::ZeroPageX, 4 };
    table[0x8D] = { InstructionType::STA, AddressingMode::Absolute,  4 };
    table[0x9D] = { InstructionType::STA, AddressingMode::AbsoluteX, 5 };
    table[0x99] = { InstructionType::STA, AddressingMode::AbsoluteY, 5 };
    table[0x81] = { InstructionType::STA, AddressingMode::IndirectX, 6 };
    table[0x91] = { InstructionType::STA, AddressingMode::IndirectY, 6 };

    // STX
    table[0x86] = { InstructionType::STX, AddressingMode::ZeroPage,  3 };
    table[0x96] = { InstructionType::STX, AddressingMode::ZeroPageY, 4 };
    table[0x8E] = { InstructionType::STX, AddressingMode::Absolute,  4 };

    // STY
    table[0x84] = { InstructionType::STY, AddressingMode::ZeroPage,  3 };
    table[0x94] = { InstructionType::STY, AddressingMode::ZeroPageX, 4 };
    table[0x8C] = { InstructionType::STY, AddressingMode::Absolute,  4 };

    // Transfers
    table[0xAA] = { InstructionType::TAX, AddressingMode::Implied, 2 };
    table[0xA8] = { InstructionType::TAY, AddressingMode::Implied, 2 };
    table[0xBA] = { InstructionType::TSX, AddressingMode::Implied, 2 };
    table[0x8A] = { InstructionType::TXA, AddressingMode::Implied, 2 };
    table[0x9A] = { InstructionType::TXS, AddressingMode::Implied, 2 };
    table[0x98] = { InstructionType::TYA, AddressingMode::Implied, 2 };

    return table;
}

}

#endif
