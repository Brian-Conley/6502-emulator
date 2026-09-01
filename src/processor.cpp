#include "processor.hpp"
#include "instruction.hpp"
#include "opcode_table.hpp"

namespace Emulator {

Mos6502::Mos6502(Bus& SystemBus):
    CumulativeCycles(0),
    SystemBus(SystemBus),
    opcodeTable(initializeOpcodeTable()),
    powered(false),
    pc(0xFFFC),
    sp(0xFF),
    sr(0b00100000),
    a(0x00),
    x(0x00),
    y(0x00)
{}

void Mos6502::step() {
    executeInstruction();
}
void Mos6502::run() {
    while (powered) {
        executeInstruction();
    }
}

void Mos6502::powerOn() {
    powered = true;
    CumulativeCycles = 0;
    sp = 0xFF;
    sr = 0b00100000;
    a = 0x00;
    x = 0x00;
    y = 0x00;
}

void Mos6502::powerOff() { powered = false; }

bool Mos6502::powerStatus() { return powered; }

void Mos6502::reset() {
    Word pch = readByte(0xFFFD) << 8;
    Word pcl = readByte(0xFFFC);
    pc = pch | pcl;
}

void Mos6502::irq() {
    Word pch = readByte(0xFFFF) << 8;
    Word pcl = readByte(0xFFFE);
    pc = pch | pcl;
}

void Mos6502::nmi() {
    Word pch = readByte(0xFFFB) << 8;
    Word pcl = readByte(0xFFFA);
    pc = pch | pcl;
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
    writeByte(sp, value);
    --sp;
}

Byte Mos6502::stackPull() {
    auto value = readByte(sp);
    ++sp;
    return value;
}

Byte Mos6502::readByte(Word address) { return SystemBus.read(address); }

void Mos6502::writeByte(Word address, Byte value) { SystemBus.write(address, value); }

Word Mos6502::readWord(Word addressLow) { return (SystemBus.read(addressLow) << 8) + SystemBus.read(addressLow + 1); }

void Mos6502::writeWord(Word addressLow, Word value) {
    SystemBus.write(addressLow, value >> 8);
    SystemBus.write(addressLow + 1, (value >> 8) & 0xFF);
}

void Mos6502::executeInstruction() {
    Byte opc = readByte(pc);
    ++pc;

    Instruction ins = opcodeTable[opc];
    switch(ins.type) {
        case InstructionType::ADC: opADC(ins.mode); break;
        case InstructionType::AND: opAND(ins.mode); break;
        case InstructionType::ASL: opASL(ins.mode); break;
        case InstructionType::BCC: opBCC(ins.mode); break;
        case InstructionType::BCS: opBCS(ins.mode); break;
        case InstructionType::BEQ: opBEQ(ins.mode); break;
        case InstructionType::BIT: opBIT(ins.mode); break;
        case InstructionType::BMI: opBMI(ins.mode); break;
        case InstructionType::BNE: opBNE(ins.mode); break;
        case InstructionType::BPL: opBPL(ins.mode); break;
        case InstructionType::BRK: opBRK(ins.mode); break;
        case InstructionType::BVC: opBVC(ins.mode); break;
        case InstructionType::BVS: opBVS(ins.mode); break;
        case InstructionType::CLC: opCLC(ins.mode); break;
        case InstructionType::CLD: opCLD(ins.mode); break;
        case InstructionType::CLI: opCLI(ins.mode); break;
        case InstructionType::CLV: opCLV(ins.mode); break;
        case InstructionType::CMP: opCMP(ins.mode); break;
        case InstructionType::CPX: opCPX(ins.mode); break;
        case InstructionType::CPY: opCPY(ins.mode); break;
        case InstructionType::DEC: opDEC(ins.mode); break;
        case InstructionType::DEX: opDEX(ins.mode); break;
        case InstructionType::DEY: opDEY(ins.mode); break;
        case InstructionType::EOR: opEOR(ins.mode); break;
        case InstructionType::INC: opINC(ins.mode); break;
        case InstructionType::INX: opINX(ins.mode); break;
        case InstructionType::INY: opINY(ins.mode); break;
        case InstructionType::JMP: opJMP(ins.mode); break;
        case InstructionType::JSR: opJSR(ins.mode); break;
        case InstructionType::LDA: opLDA(ins.mode); break;
        case InstructionType::LDX: opLDX(ins.mode); break;
        case InstructionType::LDY: opLDY(ins.mode); break;
        case InstructionType::LSR: opLSR(ins.mode); break;
        case InstructionType::NOP: opNOP(ins.mode); break;
        case InstructionType::ORA: opORA(ins.mode); break;
        case InstructionType::PHA: opPHA(ins.mode); break;
        case InstructionType::PHP: opPHP(ins.mode); break;
        case InstructionType::PLA: opPLA(ins.mode); break;
        case InstructionType::PLP: opPLP(ins.mode); break;
        case InstructionType::ROL: opROL(ins.mode); break;
        case InstructionType::ROR: opROR(ins.mode); break;
        case InstructionType::RTI: opRTI(ins.mode); break;
        case InstructionType::RTS: opRTS(ins.mode); break;
        case InstructionType::SBC: opSBC(ins.mode); break;
        case InstructionType::SEC: opSEC(ins.mode); break;
        case InstructionType::SED: opSED(ins.mode); break;
        case InstructionType::SEI: opSEI(ins.mode); break;
        case InstructionType::STA: opSTA(ins.mode); break;
        case InstructionType::STX: opSTX(ins.mode); break;
        case InstructionType::STY: opSTY(ins.mode); break;
        case InstructionType::TAX: opTAX(ins.mode); break;
        case InstructionType::TAY: opTAY(ins.mode); break;
        case InstructionType::TSX: opTSX(ins.mode); break;
        case InstructionType::TXA: opTXA(ins.mode); break;
        case InstructionType::TXS: opTXS(ins.mode); break;
        case InstructionType::TYA: opTYA(ins.mode); break;
        case InstructionType::Unknown: opUnknown(ins.mode); break;
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
    Word address = (readByte(operand + 1) << 8) + readByte(operand) + y;
    return { address, false };
}

Mos6502::ResolvedAddress Mos6502::addrRelative() {
    signed short operand = static_cast<signed char>(readByte(pc));
    pc++;
    Word address = pc + operand;
    return { address, false };
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
    Word sum = a + operand + getFlag(Flags::C);
    Byte sumByte = sum & 0xFF;
    setCarryFromValue(sum);
    setZeroFromValue(sumByte);
    setNegativeFromValue(sumByte);
    setOverflowFromValue(sumByte, a, operand);
    a = sumByte;
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

void Mos6502::opBCC(AddressingMode mode) {
    if (!getFlag(Flags::C)) {
        auto addr = resolveAddress(mode);
        if (addr.pageCrossed)
            ++CumulativeCycles;
        pc = addr.address;
    } else {
        ++pc;
    }
}

void Mos6502::opBCS(AddressingMode mode) {
    if (getFlag(Flags::C)) {
        auto addr = resolveAddress(mode);
        if (addr.pageCrossed)
            ++CumulativeCycles;
        pc = addr.address;
    } else {
        ++pc;
    }
}

void Mos6502::opBEQ(AddressingMode mode) {
    if (getFlag(Flags::Z)) {
        auto addr = resolveAddress(mode);
        if (addr.pageCrossed)
            ++CumulativeCycles;
        pc = addr.address;
    } else {
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

void Mos6502::opBMI(AddressingMode mode) {
    if (getFlag(Flags::N)) {
        auto addr = resolveAddress(mode);
        if (addr.pageCrossed)
            ++CumulativeCycles;
        pc = addr.address;
    } else {
        ++pc;
    }
}

void Mos6502::opBNE(AddressingMode mode) {
    if (!getFlag(Flags::Z)) {
        auto addr = resolveAddress(mode);
        if (addr.pageCrossed)
            ++CumulativeCycles;
        pc = addr.address;
    } else {
        ++pc;
    }
}

void Mos6502::opBPL(AddressingMode mode) {
    if (!getFlag(Flags::N)) {
        auto addr = resolveAddress(mode);
        if (addr.pageCrossed)
            ++CumulativeCycles;
        pc = addr.address;
    } else {
        ++pc;
    }
}

void Mos6502::opBRK(AddressingMode mode) {
    if (mode != AddressingMode::Implied) return;
    ++pc;
    stackPush(pc >> 8);
    stackPush(pc & 0xFF);
    stackPush(sr | 0b00010000);
    setFlag(Flags::I, true);
    irq();
}

void Mos6502::opBVC(AddressingMode mode) {
    if (!getFlag(Flags::V)) {
        auto addr = resolveAddress(mode);
        if (addr.pageCrossed)
            ++CumulativeCycles;
        pc = addr.address;
    } else {
        ++pc;
    }
}

void Mos6502::opBVS(AddressingMode mode) {
    if (getFlag(Flags::V)) {
        auto addr = resolveAddress(mode);
        if (addr.pageCrossed)
            ++CumulativeCycles;
        pc = addr.address;
    } else {
        ++pc;
    }
}

void Mos6502::opCLC(AddressingMode mode) {
    if (mode != AddressingMode::Implied)
        setFlag(Flags::C, false);
}

void Mos6502::opCLD(AddressingMode mode) {
    if (mode != AddressingMode::Implied)
        setFlag(Flags::D, false);
}

void Mos6502::opCLI(AddressingMode mode) {
    if (mode != AddressingMode::Implied)
        setFlag(Flags::I, false);
}

void Mos6502::opCLV(AddressingMode mode) {
    if (mode != AddressingMode::Implied)
        setFlag(Flags::V, false);
}

void Mos6502::opCMP(AddressingMode mode) {
    auto addr = resolveAddress(mode);
    if (addr.pageCrossed)
        ++CumulativeCycles;
    Word value = a - readByte(addr.address);
    setNegativeFromValue(value);
    setZeroFromValue(value);
    setCarryFromValue(value);
}

void Mos6502::opCPX(AddressingMode mode) {
    auto addr = resolveAddress(mode);
    if (addr.pageCrossed)
        ++CumulativeCycles;
    Word value = x - readByte(addr.address);
    setNegativeFromValue(value);
    setZeroFromValue(value);
    setCarryFromValue(value);
}

void Mos6502::opCPY(AddressingMode mode) {
    auto addr = resolveAddress(mode);
    if (addr.pageCrossed)
        ++CumulativeCycles;
    Word value = y - readByte(addr.address);
    setNegativeFromValue(value);
    setZeroFromValue(value);
    setCarryFromValue(value);
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

void Mos6502::opDEX(AddressingMode mode) {
    if (mode != AddressingMode::Implied) return;
    x -= 1;
    setNegativeFromValue(x);
    setZeroFromValue(x);
}

void Mos6502::opDEY(AddressingMode mode) {
    if (mode != AddressingMode::Implied) return;
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

void Mos6502::opINC(AddressingMode mode) {
    auto addr = resolveAddress(mode);
    if (addr.pageCrossed)
        ++CumulativeCycles;
    Byte value = readByte(addr.address) + 1;
    writeByte(addr.address, value);
    setNegativeFromValue(value);
    setZeroFromValue(value);
}

void Mos6502::opINX(AddressingMode mode) {
    if (mode != AddressingMode::Implied) return;
    ++x;
}

void Mos6502::opINY(AddressingMode mode) {
    if (mode != AddressingMode::Implied) return;
    ++y;
}

void Mos6502::opJMP(AddressingMode mode) {
    auto addr = resolveAddress(mode);
    if (addr.pageCrossed)
        ++CumulativeCycles;
    pc = addr.address;
}

void Mos6502::opJSR(AddressingMode mode) {
    Word stackAddr = pc + 2;
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
    if (mode == AddressingMode::A) {
        value = a;
        value = value >> 1;
        a = value;
    } else {
        auto addr = resolveAddress(mode);
        if (addr.pageCrossed)
            ++CumulativeCycles;
        value = readByte(addr.address);
        value = value >> 1;
        writeByte(addr.address, value);
    }
    setFlag(Flags::N, false);
    setZeroFromValue(value);
    setCarryFromValue(value);
}

void Mos6502::opNOP(AddressingMode mode) {
    if (mode != AddressingMode::Implied) return;
}

void Mos6502::opORA(AddressingMode mode) {
    auto addr = resolveAddress(mode);
    if (addr.pageCrossed)
        ++CumulativeCycles;
    a = a | readByte(addr.address);
    setNegativeFromValue(a);
    setZeroFromValue(a);
}

void Mos6502::opPHA(AddressingMode mode) {
    if (mode != AddressingMode::Implied) return;
    stackPush(a);
}

void Mos6502::opPHP(AddressingMode mode) {
    if (mode != AddressingMode::Implied) return;
    stackPush(sr);
}

void Mos6502::opPLA(AddressingMode mode) {
    if (mode != AddressingMode::Implied) return;
    a = stackPull();
    setNegativeFromValue(a);
    setZeroFromValue(a);
}

void Mos6502::opPLP(AddressingMode mode) {
    if (mode != AddressingMode::Implied) return;
    sr = stackPull();
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
    bool oldCarry = getFlag(Flags::C);
    bool newCarry = value & 0x80;
    value = (value << 1) | oldCarry;
    if (mode == AddressingMode::A) { a = value; }
    else { writeByte(addr, value); }
    setFlag(Flags::N, value);
    setFlag(Flags::Z, value);
    setFlag(Flags::C, newCarry);
}

void Mos6502::opROR(AddressingMode mode) {
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
    bool oldCarry = getFlag(Flags::C);
    bool newCarry = value & 0x1;
    value = (value >> 1) | (oldCarry << 7);
    if (mode == AddressingMode::A) { a = value; }
    else { writeByte(addr, value); }
    setFlag(Flags::N, value);
    setFlag(Flags::Z, value);
    setFlag(Flags::C, newCarry);
}

void Mos6502::opRTI(AddressingMode mode) {
    if (mode != AddressingMode::Implied) return;
    sr = stackPull() & 0b11101111;
    Byte pcl = stackPull();
    Byte pch = stackPull();
    pc = pcl | (pch << 8);
}

void Mos6502::opRTS(AddressingMode mode) {
    if (mode != AddressingMode::Implied) return;
    Byte pcl = stackPull();
    Byte pch = stackPull();
    pc = (pcl | (pch << 8)) + 1;
}

void Mos6502::opSBC(AddressingMode mode) {
    auto addr = resolveAddress(mode);
    if (addr.pageCrossed)
        ++CumulativeCycles;
    // The 6502 ALU performs SBC by doing an ADC with the inverted operand.
    auto operand = readByte(addr.address) ^ 0xFF;
    Word sum = a + operand + getFlag(Flags::C);
    Byte sumByte = sum & 0xFF;
    setCarryFromValue(sum);
    setZeroFromValue(sumByte);
    setNegativeFromValue(sumByte);
    setOverflowFromValue(sumByte, a, operand);
    a = sumByte;
}

void Mos6502::opSEC(AddressingMode mode) {
    if (mode != AddressingMode::Implied) return;
    setFlag(Flags::C, true);
}

void Mos6502::opSED(AddressingMode mode) {
    if (mode != AddressingMode::Implied) return;
    setFlag(Flags::D, true);
}

void Mos6502::opSEI(AddressingMode mode) {
    if (mode != AddressingMode::Implied) return;
    setFlag(Flags::I, true);
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

void Mos6502::opTAX(AddressingMode mode) {
    if (mode != AddressingMode::Implied) return;
    x = a;
    setNegativeFromValue(x);
    setZeroFromValue(x);
}

void Mos6502::opTAY(AddressingMode mode) {
    if (mode != AddressingMode::Implied) return;
    y = a;
    setNegativeFromValue(y);
    setZeroFromValue(y);
}

void Mos6502::opTSX(AddressingMode mode) {
    if (mode != AddressingMode::Implied) return;
    x = sp;
    setNegativeFromValue(x);
    setZeroFromValue(x);
}

void Mos6502::opTXA(AddressingMode mode) {
    if (mode != AddressingMode::Implied) return;
    a = x;
    setNegativeFromValue(a);
    setZeroFromValue(a);
}

void Mos6502::opTXS(AddressingMode mode) {
    if (mode != AddressingMode::Implied) return;
    sp = x;
}

void Mos6502::opTYA(AddressingMode mode) {
    if (mode != AddressingMode::Implied) return;
    a = y;
    setNegativeFromValue(a);
    setZeroFromValue(a);
}

void Mos6502::opUnknown(AddressingMode mode) {
    if (mode == AddressingMode::Unknown) return;
}

}
