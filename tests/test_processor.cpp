#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include "processor.hpp"

using namespace Emulator;

constexpr Byte FLAG_CARRY               = 0b00000001;
constexpr Byte FLAG_ZERO                = 0b00000010;
constexpr Byte FLAG_INTERRUPT_DISABLE   = 0b00000100;
constexpr Byte FLAG_DECIMAL             = 0b00001000;
constexpr Byte FLAG_BREAK               = 0b00010000;
constexpr Byte FLAG_UNUSED              = 0b00100000;
constexpr Byte FLAG_OVERFLOW            = 0b01000000;
constexpr Byte FLAG_NEGATIVE            = 0b10000000;

inline bool flag_status(Byte flag, Byte status) {
    return (status & flag) != 0;
}

struct CpuTestFixture {
    std::vector<Byte> memory;
    Mos6502 processor;

    struct State {
        uint64_t totalcycles;
        Word pc;
        Byte a;
        Byte x;
        Byte y;
        Byte stackptr;
        Byte status;
    };

    struct ProgramMemory {
        Word startAddr;
        std::initializer_list<Byte> bytes;
    };

    CpuTestFixture() :
        memory(0x10000, 0x00),
        processor(
                [this](Word addr) { return memory[addr]; },
                [this](Word addr, Byte val) { memory[addr] = val; }
                )
        {}

    void load_program(std::vector<ProgramMemory>& program) {
        if (program.empty()) return;
        // Wipe the memory
        std::ranges::fill(memory, 0x00);
        // Initialize the reset vector to the start of the first memory block
        memory[0xFFFC] = program[0].startAddr & 0xFF;
        memory[0xFFFD] = program[0].startAddr >> 8;
        for (auto& m : program) {
            std::ranges::copy(m.bytes, memory.begin() + m.startAddr);
        }
        // Initialize processor internal state
        processor.powerOn();
    }

    State state() {
        // The compiler didn't like the bytes being initialized this way in the return statement
        // So I'm initializing all of these outside of the return statement
        uint64_t cycles = processor.getCumulativeCycles();
        Word pc = processor.registerGet(Emulator::Mos6502::Register::ProgramCounter);
        Byte a = processor.registerGet(Emulator::Mos6502::Register::A);
        Byte x = processor.registerGet(Emulator::Mos6502::Register::X);
        Byte y = processor.registerGet(Emulator::Mos6502::Register::Y);
        Byte sp = processor.registerGet(Emulator::Mos6502::Register::StackPointer);
        Byte sr = processor.registerGet(Mos6502::Register::Status);
        return {
            cycles,
            pc, a, x, y, sp, sr
        };
    }
};

TEST_CASE("Instruction ADC", "[Processor][ADC]") {
    CpuTestFixture sys;

    SECTION("Addressing Modes") {
        // - Immediate
        // - Zero Page
        // - Zero Page,X
        // - Absolute
        // - Absolute,X (no page crossing)
        // - Absolute,X (with page crossing)
        // - Absolute,Y (no page crossing)
        // - Absolute,Y (with page crossing)
        // - Indirect,X
        // - Indirect,Y (no page crossing)
        // - Indirect,Y (with page crossing)
    }

    SECTION("Binary Arithmetic & Flags") {
        // - Basic addition without carry (e.g., $10 + $20 = $30, C=0, Z=0, N=0, V=0)
        // - Addition with incoming carry (A + M + 1)
        // - Zero result flag (e.g., $FF + $01 = $00, Z=1, C=1)
        // - Negative result flag (MSB set, N=1)
        // - Positive overflow (e.g., $7F + $01 = $80, V=1, N=1)
        // - Negative overflow (e.g., $80 + $80 = $00, V=1, C=1, Z=1)
        // - No overflow cases crossing sign boundaries safely
    }

    SECTION("Decimal (BCD) Mode Arithmetic") {
        // - Basic BCD addition without carry (e.g., $15 + $24 = $39, D=1)
        // - Low nibble correction (e.g., $08 + $05 = $13, triggers +$06 adjustment)
        // - High nibble correction (e.g., $50 + $50 = $00, sets Carry in BCD for > 99)
        // - BCD addition with incoming carry flag
    }
}

TEST_CASE("Instruction AND", "[Processor][AND]") {
    CpuTestFixture sys;

    SECTION("Addressing Modes") {
        // - Immediate (#$xx)
        // - Zero Page ($xx)
        // - Zero Page,X ($xx,X)
        // - Absolute ($xxxx)
        // - Absolute,X ($xxxx,X)
        // - Absolute,Y ($xxxx,Y)
        // - Indirect,X (($xx,X))
        // - Indirect,Y (($xx),Y)
    }

    SECTION("Flags") {
        // Negative flag set
        // Negative flag clear
        // Zero flag set
        // Zero flag clear
    }
}

TEST_CASE("Instruction SBC", "[Processor][SBC]") {
    CpuTestFixture sys;

    SECTION("Addressing Modes") {
        // - Immediate (#$xx)
        // - Zero Page ($xx)
        // - Zero Page,X ($xx,X)
        // - Absolute ($xxxx)
        // - Absolute,X ($xxxx,X)
        // - Absolute,Y ($xxxx,Y)
        // - Indirect,X (($xx,X))
        // - Indirect,Y (($xx),Y)
    }

    SECTION("Binary Arithmetic & Flags") {
        // - Basic subtraction with C=1 (no borrow) (e.g., $50 - $20 = $30, C=1, Z=0)
        // - Subtraction with C=0 (borrow) (A - M - 1)
        // - Zero result flag (e.g., $50 - $50 = $00, Z=1, C=1)
        // - Negative result / Borrow condition (e.g., $10 - $20 = $F0, C=0, N=1)
        // - Overflow flag (V) during signed subtraction edge cases
    }

    SECTION("Decimal (BCD) Mode Arithmetic") {
        // - Basic BCD subtraction with C=1 (e.g., $55 - $22 = $33, D=1)
        // - Low nibble underflow correction (triggers -$06 adjustment)
        // - High nibble underflow correction (triggers -$60 adjustment)
        // - BCD borrow behavior when C=0 initially
    }
}
