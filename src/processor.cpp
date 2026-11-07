#include "processor.hpp"
#include "instruction.hpp"
#include "opcode_table.hpp"

namespace Emulator {

Mos6502::Mos6502(std::function<Byte(Word)> readCallback, std::function<void(Word, Byte)> writeCallback) :
    CumulativeCycles(0),
    readCallback(readCallback),
    writeCallback(writeCallback),
    opcodeTable(initializeOpcodeTable()),
    pc(0xFFFC),
    sp(0xFF),
    sr(0b00100000),
    a(0x00),
    x(0x00),
    y(0x00),
    resetRequest(false),
    irqRequest(false),
    nmiRequest(false)
{}

void Mos6502::step() {
    if (resetRequest) {
        reset();
    } else if (nmiRequest) {
        nmi();
    } else if (irqRequest) {
        if (!getFlag(Flags::I))
            irq();
    }
    executeInstruction();
}

void Mos6502::powerOn() {
    reset();
    CumulativeCycles = 0;
    sp = 0xFF;
    sr = 0b00100000;
    a = 0x00;
    x = 0x00;
    y = 0x00;
    setFlag(Flags::I, false);
}

std::uint64_t Mos6502::getCumulativeCycles() const { return CumulativeCycles; }

void Mos6502::requestReset() { resetRequest = true; }

void Mos6502::requestIRQ() {
    irqRequest = true;
}

void Mos6502::requestNMI() { nmiRequest = true; }

void Mos6502::reset() {
    Word pch = readByte(0xFFFD) << 8;
    Word pcl = readByte(0xFFFC);
    pc = pch | pcl;
    resetRequest = false;
    irqRequest = false;
    nmiRequest = false;
    setFlag(Flags::I, true);
}

void Mos6502::irq() {
    stackPush(pc >> 8);
    stackPush(pc & 0xFF);
    stackPush(sr);
    setFlag(Flags::I, true);
    Word pch = readByte(0xFFFF) << 8;
    Word pcl = readByte(0xFFFE);
    pc = pch | pcl;
    irqRequest = false;
}

void Mos6502::nmi() {
    stackPush(pc >> 8);
    stackPush(pc & 0xFF);
    stackPush(sr);
    Word pch = readByte(0xFFFB) << 8;
    Word pcl = readByte(0xFFFA);
    pc = pch | pcl;
    nmiRequest = false;
}

void Mos6502::registerSet(Register reg, Word value) {
    switch(reg) {
        case Register::ProgramCounter: pc = value; break;
        case Register::StackPointer:   sp = value; break;
        case Register::Status:         sr = value; break;
        case Register::A:               a = value; break;
        case Register::X:               x = value; break;
        case Register::Y:               y = value; break;
    }
}

Word Mos6502::registerGet(Register reg) {
    switch(reg) {
        case Register::ProgramCounter: return pc;
        case Register::StackPointer:   return sp;
        case Register::Status:         return sr;
        case Register::A:              return a;
        case Register::X:              return x;
        case Register::Y:              return y;
    }
    return a; // Impossible to reach, this is here to just fix a build error.
}

void Mos6502::setFlag(Flags flag, bool state) {
    if (state) {
        sr |= static_cast<Byte>(flag);
    } else {
        sr &= ~static_cast<Byte>(flag);
    }
}
bool Mos6502::getFlag(Flags flag) const { return sr & static_cast<Byte>(flag); }

void Mos6502::setCarryFromValue(Word value) {
    setFlag(Flags::C, value > 0xFF);
}

void Mos6502::setZeroFromValue(Byte value) {
    setFlag(Flags::Z, value == 0x00);
}

void Mos6502::setOverflowFromValue(Byte value, Byte operand1, Byte operand2) {
    setFlag(Flags::V, ~(operand1 ^ operand2) & (operand1 ^ value) & 0x80);
}

void Mos6502::setNegativeFromValue(Byte value) {
    setFlag(Flags::N, value >= 0x80);
}

void Mos6502::stackPush(Byte value) {
    writeByte(0x100 | sp, value);
    --sp;
}

Byte Mos6502::stackPull() {
    ++sp;
    return readByte(0x100 | sp);
}

Byte Mos6502::readByte(Word address) {
    return readCallback(address);
}

void Mos6502::writeByte(Word address, Byte value) {
    writeCallback(address, value);
}

Word Mos6502::readWord(Word addressLow) {
    return (readCallback(addressLow) << 8) + readCallback(addressLow + 1);
}

void Mos6502::writeWord(Word addressLow, Word value) {
    writeCallback(addressLow, value & 0xFF);
    writeCallback(addressLow + 1, value >> 8);
}

void Mos6502::executeInstruction() {
    Byte opc = readByte(pc);
    ++pc;

    Instruction ins = opcodeTable[opc];
    switch(ins.type) {
        case InstructionType::ADC: opADC(ins.mode); break;
        case InstructionType::AND: opAND(ins.mode); break;
        case InstructionType::ASL: opASL(ins.mode); break;
        case InstructionType::BCC: opBranch(Flags::C, false); break;
        case InstructionType::BCS: opBranch(Flags::C, true); break;
        case InstructionType::BEQ: opBranch(Flags::Z, true); break;
        case InstructionType::BIT: opBIT(ins.mode); break;
        case InstructionType::BMI: opBranch(Flags::N, true); break;
        case InstructionType::BNE: opBranch(Flags::Z, false); break;
        case InstructionType::BPL: opBranch(Flags::N, false); break;
        case InstructionType::BRK: opBRK(); break;
        case InstructionType::BVC: opBranch(Flags::V, false); break;
        case InstructionType::BVS: opBranch(Flags::V, true); break;
        case InstructionType::CLC: opFlag(Flags::C, false); break;
        case InstructionType::CLD: opFlag(Flags::D, false); break;
        case InstructionType::CLI: opFlag(Flags::I, false); break;
        case InstructionType::CLV: opFlag(Flags::V, false); break;
        case InstructionType::CMP: opCMP(ins.mode); break;
        case InstructionType::CPX: opCPX(ins.mode); break;
        case InstructionType::CPY: opCPY(ins.mode); break;
        case InstructionType::DEC: opDEC(ins.mode); break;
        case InstructionType::DEX: opDEX(); break;
        case InstructionType::DEY: opDEY(); break;
        case InstructionType::EOR: opEOR(ins.mode); break;
        case InstructionType::INC: opINC(ins.mode); break;
        case InstructionType::INX: opINX(); break;
        case InstructionType::INY: opINY(); break;
        case InstructionType::JMP: opJMP(ins.mode); break;
        case InstructionType::JSR: opJSR(ins.mode); break;
        case InstructionType::LDA: opLDA(ins.mode); break;
        case InstructionType::LDX: opLDX(ins.mode); break;
        case InstructionType::LDY: opLDY(ins.mode); break;
        case InstructionType::LSR: opLSR(ins.mode); break;
        case InstructionType::NOP: opNOP(); break;
        case InstructionType::ORA: opORA(ins.mode); break;
        case InstructionType::PHA: opPHA(); break;
        case InstructionType::PHP: opPHP(); break;
        case InstructionType::PLA: opPLA(); break;
        case InstructionType::PLP: opPLP(); break;
        case InstructionType::ROL: opROL(ins.mode); break;
        case InstructionType::RTI: opRTI(); break;
        case InstructionType::RTS: opRTS(); break;
        case InstructionType::SBC: opSBC(ins.mode); break;
        case InstructionType::SEC: opFlag(Flags::C, true); break;
        case InstructionType::SED: opFlag(Flags::D, true); break;
        case InstructionType::SEI: opFlag(Flags::I, true); break;
        case InstructionType::STA: opSTA(ins.mode); break;
        case InstructionType::STX: opSTX(ins.mode); break;
        case InstructionType::STY: opSTY(ins.mode); break;
        case InstructionType::TAX: opTAX(); break;
        case InstructionType::TAY: opTAY(); break;
        case InstructionType::TSX: opTSX(); break;
        case InstructionType::TXA: opTXA(); break;
        case InstructionType::TXS: opTXS(); break;
        case InstructionType::TYA: opTYA(); break;
        case InstructionType::Unknown: opUnknown(); break;
    }
    CumulativeCycles += ins.cycles;
}

Mos6502::ResolvedAddress Mos6502::resolveAddress(AddressingMode mode) {
    switch (mode) {
        case AddressingMode::Absolute:  return addrAbsolute();
        case AddressingMode::AbsoluteX: return addrAbsoluteX();
        case AddressingMode::AbsoluteY: return addrAbsoluteY();
        case AddressingMode::Immediate: return addrImmediate();
        case AddressingMode::Indirect:  return addrIndirect();
        case AddressingMode::IndirectX: return addrIndirectX();
        case AddressingMode::IndirectY: return addrIndirectY();
        case AddressingMode::Relative:  return addrRelative();
        case AddressingMode::ZeroPage:  return addrZeroPage();
        case AddressingMode::ZeroPageX: return addrZeroPageX();
        case AddressingMode::ZeroPageY: return addrZeroPageY();
        default:                        return addrUnknown();
    }
}

Mos6502::ResolvedAddress Mos6502::addrAbsolute() {
    Word address = (readByte(pc + 1) << 8) + readByte(pc);
    pc += 2;
    return { address, false };
}

Mos6502::ResolvedAddress Mos6502::addrAbsoluteX() {
    Byte highByte = readByte(pc + 1);
    Word address = (highByte << 8) + readByte(pc) + x;
    bool pageCrossed = (address >> 8) != highByte;
    pc += 2;
    return { address, pageCrossed };
}

Mos6502::ResolvedAddress Mos6502::addrAbsoluteY() {
    Byte highByte = readByte(pc + 1);
    Word address = (highByte << 8) + readByte(pc) + y;
    bool pageCrossed = (address >> 8) != highByte;
    pc += 2;
    return { address, pageCrossed };
}

Mos6502::ResolvedAddress Mos6502::addrImmediate() {
    Word address = pc;
    pc++;
    return { address, false };
}

Mos6502::ResolvedAddress Mos6502::addrIndirect() {
    Word operand = (readByte(pc + 1) << 8) + readByte(pc);
    pc += 2;
    Word address = (readByte(operand + 1) << 8) + readByte(operand);
    return { address, false };
}

Mos6502::ResolvedAddress Mos6502::addrIndirectX() {
    Byte operand = readByte(pc) + x;
    pc++;
    Word address = (readByte(operand + 1) << 8) + readByte(operand);
    return { address, false };
}

Mos6502::ResolvedAddress Mos6502::addrIndirectY() {
    Byte operand = readByte(pc);
    pc++;
    Word baseAddress = (readByte(operand + 1) << 8) + readByte(operand);
    Word address = baseAddress + y;
    bool pageCrossed = (address >> 8) != (baseAddress >> 8);
    return { address, pageCrossed};
}

Mos6502::ResolvedAddress Mos6502::addrRelative() {
    signed short operand = static_cast<signed char>(readByte(pc));
    pc++;
    Word address = pc + operand;
    bool pageCrossed = (address >> 8) != (pc >> 8);
    return { address, pageCrossed };
}

Mos6502::ResolvedAddress Mos6502::addrZeroPage() {
    Byte address = readByte(pc);
    pc++;
    return { address, false };
}

Mos6502::ResolvedAddress Mos6502::addrZeroPageX() {
    Byte address = readByte(pc) + x;
    pc++;
    return { address, false };
}

Mos6502::ResolvedAddress Mos6502::addrZeroPageY() {
    Byte address = readByte(pc) + y;
    pc++;
    return { address, false };
}

Mos6502::ResolvedAddress Mos6502::addrUnknown() { return { 0xFFFF, false }; }

void Mos6502::opADC(AddressingMode mode) {
    auto addr = resolveAddress(mode);
    if (addr.pageCrossed)
        ++CumulativeCycles;
    auto operand = readByte(addr.address);
    Byte sum = 0x00;
    bool carry_in = getFlag(Flags::C);
    bool carry_out = false;

    if (getFlag(Flags::D)) {
        // Decimal mode
        Byte a_lo = a & 0x0F;
        Byte a_hi = a >> 4;
        Byte op_lo = operand & 0x0F;
        Byte op_hi = operand >> 4;

        Byte result_lo = a_lo + op_lo + carry_in;
        Byte result_lo_carry = 0x00;
        if (result_lo > 0x09)
            result_lo += 0x06;
        if (result_lo > 0x0F) {
            result_lo_carry = 0x01;
            result_lo &= 0x0F;
        }

        Byte result_hi = a_hi + op_hi + result_lo_carry;
        if (result_hi > 0x09) {
            result_hi += 0x06;
        }
        if (result_hi > 0x0F) {
            result_hi &= 0x0F;
            carry_out = true;
        }

        sum = (result_hi << 4) | result_lo;

    } else {
        // Not decimal mode
        Word sum_word = a + operand + carry_in;
        sum = sum_word & 0xFF;
        carry_out = sum_word > 0xFF;
    }

    setCarryFromValue(carry_out << 8);
    setZeroFromValue(sum);
    setNegativeFromValue(sum);
    setOverflowFromValue(sum, a, operand);

    a = sum;
}

void Mos6502::opAND(AddressingMode mode) {
    auto addr = resolveAddress(mode);
    if (addr.pageCrossed)
        ++CumulativeCycles;
    auto operand = readByte(addr.address);
    a = a & operand;
    setZeroFromValue(a);
    setNegativeFromValue(a);
}

void Mos6502::opASL(AddressingMode mode) {
    Word value;
    if (mode == AddressingMode::A) {
        value = a;
        value = value << 1;
        a = value;
    } else {
        auto addr = resolveAddress(mode);
        if (addr.pageCrossed)
            ++CumulativeCycles;
        value = readByte(addr.address);
        value = value << 1;
        writeByte(addr.address, value);
    }
    setNegativeFromValue(value);
    setZeroFromValue(value);
    setCarryFromValue(value);
}

void Mos6502::opBranch(Flags flag, bool branchCondition) {
    // BCC, BCS, BEQ, BNE, BMI, BPL, BVC, BVS
    if (getFlag(flag) == branchCondition) {
        // Branch taken
        ++CumulativeCycles;
        auto addr = resolveAddress(AddressingMode::Relative);
        if (addr.pageCrossed)
            ++CumulativeCycles;
        pc = addr.address;
    } else {
        // Branch not taken
        ++pc;
    }
}

void Mos6502::opBIT(AddressingMode mode) {
    auto addr = resolveAddress(mode);
    if (addr.pageCrossed)
        ++CumulativeCycles;
    auto value = readByte(addr.address);
    setZeroFromValue(value & a);
    setFlag(Flags::N, value & 0x80);
    setFlag(Flags::V, value & 0x40);
}

void Mos6502::opBRK() {
    ++pc;
    stackPush(pc >> 8);
    stackPush(pc & 0xFF);
    stackPush(sr | 0b00110000);
    setFlag(Flags::I, true);
    pc = readByte(0xFFFE) | (readByte(0xFFFF) << 8);
}

void Mos6502::opCMP(AddressingMode mode) {
    auto addr = resolveAddress(mode);
    if (addr.pageCrossed)
        ++CumulativeCycles;
    Byte value = readByte(addr.address);
    setNegativeFromValue(a - value);
    setFlag(Flags::Z, a == value ? true : false);
    setFlag(Flags::C, a >= value ? true : false);
}

void Mos6502::opCPX(AddressingMode mode) {
    auto addr = resolveAddress(mode);
    if (addr.pageCrossed)
        ++CumulativeCycles;
    Byte value = readByte(addr.address);
    setNegativeFromValue(x - value);
    setFlag(Flags::Z, x == value ? true : false);
    setFlag(Flags::C, x >= value ? true : false);
}

void Mos6502::opCPY(AddressingMode mode) {
    auto addr = resolveAddress(mode);
    if (addr.pageCrossed)
        ++CumulativeCycles;
    Byte value = readByte(addr.address);
    setNegativeFromValue(y - value);
    setFlag(Flags::Z, y == value ? true : false);
    setFlag(Flags::C, y >= value ? true : false);
}

void Mos6502::opDEC(AddressingMode mode) {
    auto addr = resolveAddress(mode);
    if (addr.pageCrossed)
        ++CumulativeCycles;
    Byte value = readByte(addr.address) - 1;
    writeByte(addr.address, value);
    setNegativeFromValue(value);
    setZeroFromValue(value);
}

void Mos6502::opDEX() {
    x -= 1;
    setNegativeFromValue(x);
    setZeroFromValue(x);
}

void Mos6502::opDEY() {
    y -= 1;
    setNegativeFromValue(y);
    setZeroFromValue(y);
}

void Mos6502::opEOR(AddressingMode mode) {
    auto addr = resolveAddress(mode);
    if (addr.pageCrossed)
        ++CumulativeCycles;
    Byte value = readByte(addr.address);
    a = a ^ value;
    setNegativeFromValue(a);
    setZeroFromValue(a);
}

void Mos6502::opFlag(Flags flag, bool state) {
    setFlag(flag, state);
}

void Mos6502::opINC(AddressingMode mode) {
    auto addr = resolveAddress(mode);
    if (addr.pageCrossed)
        ++CumulativeCycles;
    Byte value = readByte(addr.address) + 1;
    writeByte(addr.address, value);
    setNegativeFromValue(value);
    setZeroFromValue(value);
}

void Mos6502::opINX() {
    ++x;
    setNegativeFromValue(x);
    setZeroFromValue(x);
}

void Mos6502::opINY() {
    ++y;
    setNegativeFromValue(y);
    setZeroFromValue(y);
}

void Mos6502::opJMP(AddressingMode mode) {
    auto addr = resolveAddress(mode);
    if (addr.pageCrossed)
        ++CumulativeCycles;
    pc = addr.address;
}

void Mos6502::opJSR(AddressingMode mode) {
    Word stackAddr = pc + 1;
    stackPush((stackAddr >> 8) & 0xFF);
    stackPush(stackAddr & 0xFF);
    auto addr = resolveAddress(mode);
    if (addr.pageCrossed)
        ++CumulativeCycles;
    pc = addr.address;
}

void Mos6502::opLDA(AddressingMode mode) {
    auto addr = resolveAddress(mode);
    if (addr.pageCrossed)
        ++CumulativeCycles;
    a = readByte(addr.address);
    setNegativeFromValue(a);
    setZeroFromValue(a);
}

void Mos6502::opLDX(AddressingMode mode) {
    auto addr = resolveAddress(mode);
    if (addr.pageCrossed)
        ++CumulativeCycles;
    x = readByte(addr.address);
    setNegativeFromValue(x);
    setZeroFromValue(x);
}

void Mos6502::opLDY(AddressingMode mode) {
    auto addr = resolveAddress(mode);
    if (addr.pageCrossed)
        ++CumulativeCycles;
    y = readByte(addr.address);
    setNegativeFromValue(y);
    setZeroFromValue(y);
}

void Mos6502::opLSR(AddressingMode mode) {
    Word value;
    bool carry_out;
    if (mode == AddressingMode::A) {
        value = a;
        carry_out = (value & 0x01) != 0;
        value = value >> 1;
        a = value;
    } else {
        auto addr = resolveAddress(mode);
        if (addr.pageCrossed)
            ++CumulativeCycles;
        value = readByte(addr.address);
        carry_out = (value & 0x01) != 0;
        value = value >> 1;
        writeByte(addr.address, value);
    }
    setFlag(Flags::N, false);
    setZeroFromValue(value);
    setFlag(Flags::C, carry_out);
}

void Mos6502::opNOP() {
    return;
}

void Mos6502::opORA(AddressingMode mode) {
    auto addr = resolveAddress(mode);
    if (addr.pageCrossed)
        ++CumulativeCycles;
    a = a | readByte(addr.address);
    setNegativeFromValue(a);
    setZeroFromValue(a);
}

void Mos6502::opPHA() {
    stackPush(a);
}

void Mos6502::opPHP() {
    stackPush(sr | 0b00110000);
}

void Mos6502::opPLA() {
    a = stackPull();
    setNegativeFromValue(a);
    setZeroFromValue(a);
}

void Mos6502::opPLP() {
    sr = stackPull() & 0b11101111;
}

void Mos6502::opROL(AddressingMode mode) {
    Byte value;
    Word addr;
    if (mode == AddressingMode::A) { value = a; }
    else {
        auto ra = resolveAddress(mode);
        if (ra.pageCrossed)
            ++CumulativeCycles;
        addr = ra.address;
        value = readByte(addr);
    }
    bool carry_in = getFlag(Flags::C);
    bool carry_out = (value & 0x80) != 0;
    value = (value << 1) | carry_in;
    if (mode == AddressingMode::A) { a = value; }
    else { writeByte(addr, value); }
    setNegativeFromValue(value);
    setZeroFromValue(value);
    setFlag(Flags::C, carry_out);
}

void Mos6502::opRTI() {
    sr = stackPull() & 0b11101111;
    Byte pcl = stackPull();
    Byte pch = stackPull();
    pc = pcl | (pch << 8);
}

void Mos6502::opRTS() {
    Byte pcl = stackPull();
    Byte pch = stackPull();
    pc = (pcl | (pch << 8)) + 1;
}

void Mos6502::opSBC(AddressingMode mode) {
    auto addr = resolveAddress(mode);
    if (addr.pageCrossed)
        ++CumulativeCycles;
    auto operand = readByte(addr.address);

    Byte sum = 0x00;
    bool carry_in = getFlag(Flags::C);
    bool carry_out = true;

    if (getFlag(Flags::D)) {
        // Decimal mode
        Byte borrow_in = 1 - carry_in;
        Byte a_lo = a & 0x0F;
        Byte op_lo = operand & 0x0F;
        Byte result_lo;
        Byte result_lo_borrow = 0x00;
        if (a_lo < (op_lo + borrow_in)) {
            result_lo = a_lo + 0x0A - op_lo - borrow_in;
            result_lo_borrow = 0x01;
        } else {
            result_lo = a_lo - op_lo - borrow_in;
        }

        Byte borrow_out = 0x00;
        Byte result_hi;
        Byte a_hi = a >> 4;
        Byte op_hi = operand >> 4;
        if (a_hi < (op_hi + result_lo_borrow)) {
            result_hi = a_hi + 0x0A - op_hi - result_lo_borrow;
            borrow_out = 0x01;
        } else {
            result_hi = a_hi - op_hi - result_lo_borrow;
        }
        carry_out = (borrow_out == 0x00);

        sum = (result_hi << 4) | result_lo;

    } else {
        // Not decimal mode
        // The 6502 ALU performs SBC by doing an ADC with the inverted operand.
        Word sum_word = a + (operand ^ 0xFF) + carry_in;
        sum = sum_word & 0xFF;
        carry_out = sum_word > 0xFF;
    }

    setCarryFromValue(carry_out << 8);
    setZeroFromValue(sum);
    setNegativeFromValue(sum);
    setFlag(Flags::V, ((a ^ (operand ^ 0xFF)) & (a ^ sum) & 0x80) != 0);

    a = sum;
}

void Mos6502::opSTA(AddressingMode mode) {
    auto addr = resolveAddress(mode);
    if (addr.pageCrossed)
        ++CumulativeCycles;
    writeByte(addr.address, a);
}

void Mos6502::opSTX(AddressingMode mode) {
    auto addr = resolveAddress(mode);
    if (addr.pageCrossed)
        ++CumulativeCycles;
    writeByte(addr.address, x);
}

void Mos6502::opSTY(AddressingMode mode) {
    auto addr = resolveAddress(mode);
    if (addr.pageCrossed)
        ++CumulativeCycles;
    writeByte(addr.address, y);
}

void Mos6502::opTAX() {
    x = a;
    setNegativeFromValue(x);
    setZeroFromValue(x);
}

void Mos6502::opTAY() {
    y = a;
    setNegativeFromValue(y);
    setZeroFromValue(y);
}

void Mos6502::opTSX() {
    x = sp;
    setNegativeFromValue(x);
    setZeroFromValue(x);
}

void Mos6502::opTXA() {
    a = x;
    setNegativeFromValue(a);
    setZeroFromValue(a);
}

void Mos6502::opTXS() {
    sp = x;
}

void Mos6502::opTYA() {
    a = y;
    setNegativeFromValue(a);
    setZeroFromValue(a);
}

void Mos6502::opUnknown() {
    return;
}

}
