#ifndef PROCESSOR_HPP
#define PROCESSOR_HPP

#include "bus.hpp"
#include "bytes.hpp"
#include "instruction.hpp"
#include <array>

namespace Emulator {

class Mos6502 {
    public:
        Mos6502(Bus& SystemBus);

        void step(); /**< Execute one instruction. */
        void run();  /**< Continuously execute instructions until termination. */

        void reset();
        void irq();
        void nmi();

        /** @name Register Accessors (debug/testing only) */
        ///@{
        enum class Register { ProgramCounter, StackPointer, Status, A, X, Y };
        void registerSet(Register reg, Word value); /**< 8-bit registers will only use the low byte of value. */
        Word registerGet(Register reg);
        ///}

    private:
        Mos6502() = delete;

        unsigned int CumulativeCycles; /**< Used for controlling the run speed. */
        Bus& SystemBus; /**< Handles memory reads/writes for memory and devices. */
        const std::array<Instruction, 256> opcodeTable; /**< Lookup table for instructions. */

        /** @name Registers */
        ///@{
        Word pc; /**< Program counter */
        Byte sp; /**< Stack pointer */
        Byte sr; /**< Status register (NV-BDIZC) */
        Byte a;  /**< Accumulator */
        Byte x;  /**< X-index register */
        Byte y;  /**< Y-index register */
        ///@}

        /** @name Status Register Helpers */
        ///@{
        enum Flags : Byte {
            C = (1 << 0), // Carry
            Z = (1 << 1), // Zero
            I = (1 << 2), // Interrupt Disable
            D = (1 << 3), // Decimal
            B = (1 << 4), // Break
            U = (1 << 5), // Unused
            V = (1 << 6), // Overflow
            N = (1 << 7)  // Negative
        };
        void setFlag(Flags flag, bool state);
        bool getFlag(Flags flag) const;
        ///@}

        /** @name Memory Operations */
        ///@{
        Byte readByte(Word address);                 /**< Read an 8-bit value from address. */
        void writeByte(Word address, Byte value);    /**< Write an 8-bit value to address. */
        Word readWord(Word addressLow);              /**< Read a 16-bit value from addressLow and addressLow+1. */
        void writeWord(Word addressLow, Word value); /**< Write a 16-bit value to addressLow and addressLow+1. */
        ///@}

        void executeInstruction(); /**< Fetch, decode, and execute the instruction at 0xPC, increments PC accordingly. */

        /** @name Addressing Modes */
        ///@{
        struct ResolvedAddress {
            Word address;
            bool pageCrossed;
        };
        ResolvedAddress resolveAddress(AddressingMode mode); /**< Helper to dispatch the correct addressing mode. */
        // Word addrA();
        Word addrAbsolute();
        Word addrAbsoluteX();
        Word addrAbsoluteY();
        Word addrImmediate();
        // Word addrImplied();
        Word addrIndirect();
        Word addrIndirectX();
        Word addrIndirectY();
        Word addrRelative();
        Word addrZeroPage();
        Word addrZeroPageX();
        Word addrZeroPageY();
        Word addrUnknown();
        ///@}

        /** @name Instructions */
        ///@{
        void opADC(AddressingMode mode, unsigned int baseCycles);
        void opAND(AddressingMode mode, unsigned int baseCycles);
        void opASL(AddressingMode mode, unsigned int baseCycles);
        void opBCC(AddressingMode mode, unsigned int baseCycles);
        void opBCS(AddressingMode mode, unsigned int baseCycles);
        void opBEQ(AddressingMode mode, unsigned int baseCycles);
        void opBIT(AddressingMode mode, unsigned int baseCycles);
        void opBMI(AddressingMode mode, unsigned int baseCycles);
        void opBNE(AddressingMode mode, unsigned int baseCycles);
        void opBPL(AddressingMode mode, unsigned int baseCycles);
        void opBRK(AddressingMode mode, unsigned int baseCycles);
        void opBVC(AddressingMode mode, unsigned int baseCycles);
        void opBVS(AddressingMode mode, unsigned int baseCycles);
        void opCLC(AddressingMode mode, unsigned int baseCycles);
        void opCLD(AddressingMode mode, unsigned int baseCycles);
        void opCLI(AddressingMode mode, unsigned int baseCycles);
        void opCLV(AddressingMode mode, unsigned int baseCycles);
        void opCMP(AddressingMode mode, unsigned int baseCycles);
        void opCPX(AddressingMode mode, unsigned int baseCycles);
        void opCPY(AddressingMode mode, unsigned int baseCycles);
        void opDEC(AddressingMode mode, unsigned int baseCycles);
        void opDEX(AddressingMode mode, unsigned int baseCycles);
        void opDEY(AddressingMode mode, unsigned int baseCycles);
        void opEOR(AddressingMode mode, unsigned int baseCycles);
        void opINC(AddressingMode mode, unsigned int baseCycles);
        void opINX(AddressingMode mode, unsigned int baseCycles);
        void opINY(AddressingMode mode, unsigned int baseCycles);
        void opJMP(AddressingMode mode, unsigned int baseCycles);
        void opJSR(AddressingMode mode, unsigned int baseCycles);
        void opLDA(AddressingMode mode, unsigned int baseCycles);
        void opLDX(AddressingMode mode, unsigned int baseCycles);
        void opLDY(AddressingMode mode, unsigned int baseCycles);
        void opLSR(AddressingMode mode, unsigned int baseCycles);
        void opNOP(AddressingMode mode, unsigned int baseCycles);
        void opORA(AddressingMode mode, unsigned int baseCycles);
        void opPHA(AddressingMode mode, unsigned int baseCycles);
        void opPHP(AddressingMode mode, unsigned int baseCycles);
        void opPLA(AddressingMode mode, unsigned int baseCycles);
        void opPLP(AddressingMode mode, unsigned int baseCycles);
        void opROL(AddressingMode mode, unsigned int baseCycles);
        void opROR(AddressingMode mode, unsigned int baseCycles);
        void opRTI(AddressingMode mode, unsigned int baseCycles);
        void opRTS(AddressingMode mode, unsigned int baseCycles);
        void opSBC(AddressingMode mode, unsigned int baseCycles);
        void opSEC(AddressingMode mode, unsigned int baseCycles);
        void opSED(AddressingMode mode, unsigned int baseCycles);
        void opSEI(AddressingMode mode, unsigned int baseCycles);
        void opSTA(AddressingMode mode, unsigned int baseCycles);
        void opSTX(AddressingMode mode, unsigned int baseCycles);
        void opSTY(AddressingMode mode, unsigned int baseCycles);
        void opTAX(AddressingMode mode, unsigned int baseCycles);
        void opTAY(AddressingMode mode, unsigned int baseCycles);
        void opTSX(AddressingMode mode, unsigned int baseCycles);
        void opTXA(AddressingMode mode, unsigned int baseCycles);
        void opTXS(AddressingMode mode, unsigned int baseCycles);
        void opTYA(AddressingMode mode, unsigned int baseCycles);
        void opUnknown(AddressingMode mode, unsigned int baseCycles);
        ///@}

};

}

#endif
