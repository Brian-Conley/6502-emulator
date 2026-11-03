#ifndef INSTRUCTION_HPP
#define INSTRUCTION_HPP

namespace Emulator {

enum class InstructionType {
    ADC, AND, ASL, BCC, BCS, BEQ, BIT, BMI,
    BNE, BPL, BRK, BVC, BVS, CLC, CLD, CLI,
    CLV, CMP, CPX, CPY, DEC, DEX, DEY, EOR,
    INC, INX, INY, JMP, JSR, LDA, LDX, LDY,
    LSR, NOP, ORA, PHA, PHP, PLA, PLP, ROL,
    RTI, RTS, SBC, SEC, SED, SEI, STA,
    STX, STY, TAX, TAY, TSX, TXA, TXS, TYA,
    Unknown,
};

enum class AddressingMode {
    A, Absolute, AbsoluteX, AbsoluteY,
    Immediate, Implied, Indirect, IndirectX,
    IndirectY, Relative, ZeroPage, ZeroPageX,
    ZeroPageY, Unknown,
};

struct Instruction {
    InstructionType type;
    AddressingMode mode;
    unsigned int cycles;
};

};

#endif
