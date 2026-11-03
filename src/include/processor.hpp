#ifndef PROCESSOR_HPP
#define PROCESSOR_HPP

#include "bytes.hpp"
#include "instruction.hpp"
#include <array>
#include <cstdint>
#include "functional"

namespace Emulator {

class Mos6502 {
    public:
        Mos6502(std::function<Byte(Word)> readCallback, std::function<void(Word, Byte)> writeCallback);

        void step(); /**< Execute one instruction. */
        void powerOn();  /**< Full state reset. */
        std::uint64_t getCumulativeCycles() const; /**< Public accessor to get the cumulative cycles. */

        /** @Interrupt requests */
        ///@{
        void requestReset();
        void requestIRQ();
        void requestNMI();
        ///@}

        /** @name Register Accessors (debug/testing only) */
        ///@{
        enum class Register { ProgramCounter, StackPointer, Status, A, X, Y };
        void registerSet(Register reg, Word value); /**< 8-bit registers will only use the low byte of value. */
        Word registerGet(Register reg);
        ///}

    private:
        Mos6502() = delete;

        std::uint64_t CumulativeCycles; /**< Used for controlling the run speed. */
        std::function<Byte(Word)> readCallback; /**< Interface for reading from the address space. */
        std::function<void(Word, Byte)> writeCallback; /**< Interface for writing to the address space. */
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
        enum class Flags : Byte {
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
        void setCarryFromValue(Word value);
        void setZeroFromValue(Byte value);
        void setOverflowFromValue(Byte value, Byte operand1, Byte operand2);
        void setNegativeFromValue(Byte value);
        ///@}

        /** @name Stack Operations */
        ///@{
        void stackPush(Byte value);
        Byte stackPull();
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
        // ResolvedAddress addrA();
        ResolvedAddress addrAbsolute();
        ResolvedAddress addrAbsoluteX();
        ResolvedAddress addrAbsoluteY();
        ResolvedAddress addrImmediate();
        // ResolvedAddress addrImplied();
        ResolvedAddress addrIndirect();
        ResolvedAddress addrIndirectX();
        ResolvedAddress addrIndirectY();
        ResolvedAddress addrRelative();
        ResolvedAddress addrZeroPage();
        ResolvedAddress addrZeroPageX();
        ResolvedAddress addrZeroPageY();
        ResolvedAddress addrUnknown();
        ///@}

        /** @name Instructions */
        ///@{
        void opADC(AddressingMode mode);
        void opAND(AddressingMode mode);
        void opASL(AddressingMode mode);
        void opBranch(Flags flag, bool branchCondition);
        void opBIT(AddressingMode mode);
        void opBRK();
        void opCMP(AddressingMode mode);
        void opCPX(AddressingMode mode);
        void opCPY(AddressingMode mode);
        void opDEC(AddressingMode mode);
        void opDEX();
        void opDEY();
        void opEOR(AddressingMode mode);
        void opFlag(Flags flag, bool state);
        void opINC(AddressingMode mode);
        void opINX();
        void opINY();
        void opJMP(AddressingMode mode);
        void opJSR(AddressingMode mode);
        void opLDA(AddressingMode mode);
        void opLDX(AddressingMode mode);
        void opLDY(AddressingMode mode);
        void opLSR(AddressingMode mode);
        void opNOP();
        void opORA(AddressingMode mode);
        void opPHA();
        void opPHP();
        void opPLA();
        void opPLP();
        void opROL(AddressingMode mode);
        void opRTI();
        void opRTS();
        void opSBC(AddressingMode mode);
        void opSTA(AddressingMode mode);
        void opSTX(AddressingMode mode);
        void opSTY(AddressingMode mode);
        void opTAX();
        void opTAY();
        void opTSX();
        void opTXA();
        void opTXS();
        void opTYA();
        void opUnknown();
        ///@}

        void reset();
        void irq();
        void nmi();
        bool resetRequest;
        bool irqRequest;
        bool nmiRequest;

};

}

#endif
