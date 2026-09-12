#ifndef PROCESSOR_HPP
#define PROCESSOR_HPP

#include "bus.hpp"
#include "bytes.hpp"
#include "instruction.hpp"
#include <array>
#include <atomic>
#include <memory>

namespace Emulator {

class Mos6502 {
    public:
        Mos6502(std::shared_ptr<Bus> SystemBus);

        void step(); /**< Execute one instruction. */
        void run();  /**< Continuously execute instructions until termination. */

        void powerOn();  /**< Set powered flag. */
        void powerOff(); /**< Clear powered flag. */
        bool powerStatus(); /**< Get powered flag. */

        /** @Interrupr requests */
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

        unsigned int CumulativeCycles; /**< Used for controlling the run speed. */
        std::shared_ptr<Bus> SystemBus; /**< Handles memory reads/writes for memory and devices. */
        const std::array<Instruction, 256> opcodeTable; /**< Lookup table for instructions. */
        std::atomic<bool> powered; /**< Determine if instructions should be running. */

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
        void opBCC(AddressingMode mode);
        void opBCS(AddressingMode mode);
        void opBEQ(AddressingMode mode);
        void opBIT(AddressingMode mode);
        void opBMI(AddressingMode mode);
        void opBNE(AddressingMode mode);
        void opBPL(AddressingMode mode);
        void opBRK(AddressingMode mode);
        void opBVC(AddressingMode mode);
        void opBVS(AddressingMode mode);
        void opCLC(AddressingMode mode);
        void opCLD(AddressingMode mode);
        void opCLI(AddressingMode mode);
        void opCLV(AddressingMode mode);
        void opCMP(AddressingMode mode);
        void opCPX(AddressingMode mode);
        void opCPY(AddressingMode mode);
        void opDEC(AddressingMode mode);
        void opDEX(AddressingMode mode);
        void opDEY(AddressingMode mode);
        void opEOR(AddressingMode mode);
        void opINC(AddressingMode mode);
        void opINX(AddressingMode mode);
        void opINY(AddressingMode mode);
        void opJMP(AddressingMode mode);
        void opJSR(AddressingMode mode);
        void opLDA(AddressingMode mode);
        void opLDX(AddressingMode mode);
        void opLDY(AddressingMode mode);
        void opLSR(AddressingMode mode);
        void opNOP(AddressingMode mode);
        void opORA(AddressingMode mode);
        void opPHA(AddressingMode mode);
        void opPHP(AddressingMode mode);
        void opPLA(AddressingMode mode);
        void opPLP(AddressingMode mode);
        void opROL(AddressingMode mode);
        void opROR(AddressingMode mode);
        void opRTI(AddressingMode mode);
        void opRTS(AddressingMode mode);
        void opSBC(AddressingMode mode);
        void opSEC(AddressingMode mode);
        void opSED(AddressingMode mode);
        void opSEI(AddressingMode mode);
        void opSTA(AddressingMode mode);
        void opSTX(AddressingMode mode);
        void opSTY(AddressingMode mode);
        void opTAX(AddressingMode mode);
        void opTAY(AddressingMode mode);
        void opTSX(AddressingMode mode);
        void opTXA(AddressingMode mode);
        void opTXS(AddressingMode mode);
        void opTYA(AddressingMode mode);
        void opUnknown(AddressingMode mode);
        ///@}

        void reset();
        void irq();
        void nmi();
        std::atomic<bool> resetRequest;
        std::atomic<bool> irqRequest;
        std::atomic<bool> nmiRequest;

};

}

#endif
