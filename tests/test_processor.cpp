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

    void load_program(const std::vector<ProgramMemory>& program) {
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

/******************************************
 * Group: Conditional Branch Instructions *
 * BCC, BCS, BEQ, BMI, BNE, BPL, BVC, BVS *
 ******************************************/

TEST_CASE("Instruction BCC", "[Processor][BCC]") {
    CpuTestFixture sys;
    Byte opcode = 0x90;

    SECTION("Branch not taken") {
        sys.load_program({
            { 0x8000, {opcode, 0x20} }
        });
        sys.processor.registerSet(Mos6502::Register::Status, 0x00 | FLAG_CARRY);
        sys.processor.step();
        auto state = sys.state();
        REQUIRE(state.pc == 0x8002);
        REQUIRE(state.totalcycles == 2);
    }

    SECTION("Branch taken (no page crossing)") {
        sys.load_program({
            { 0x8000, {opcode, 0x20} }
        });
        sys.processor.registerSet(Mos6502::Register::Status, 0x00);
        sys.processor.step();
        auto state = sys.state();
        REQUIRE(state.pc == 0x8022);
        REQUIRE(state.totalcycles == 3);
    }

    SECTION("Branch taken (with page crossing)") {
        sys.load_program({
            { 0x80C0, {opcode, 0x40} }
        });
        sys.processor.registerSet(Mos6502::Register::Status, 0x00);
        sys.processor.step();
        auto state = sys.state();
        REQUIRE(state.pc == 0x8102);
        REQUIRE(state.totalcycles == 4);
    }
}

TEST_CASE("Instruction BCS", "[Processor][BCS]") {
    CpuTestFixture sys;
    Byte opcode = 0xB0;

    SECTION("Branch not taken") {
        sys.load_program({
            { 0x8000, {opcode, 0x20} }
        });
        sys.processor.registerSet(Mos6502::Register::Status, 0x00);
        sys.processor.step();
        auto state = sys.state();
        REQUIRE(state.pc == 0x8002);
        REQUIRE(state.totalcycles == 2);
    }

    SECTION("Branch taken (no page crossing)") {
        sys.load_program({
            { 0x8000, {opcode, 0x20} }
        });
        sys.processor.registerSet(Mos6502::Register::Status, 0x00 | FLAG_CARRY);
        sys.processor.step();
        auto state = sys.state();
        REQUIRE(state.pc == 0x8022);
        REQUIRE(state.totalcycles == 3);
    }

    SECTION("Branch taken (with page crossing)") {
        sys.load_program({
            { 0x80C0, {opcode, 0x40} }
        });
        sys.processor.registerSet(Mos6502::Register::Status, 0x00 | FLAG_CARRY);
        sys.processor.step();
        auto state = sys.state();
        REQUIRE(state.pc == 0x8102);
        REQUIRE(state.totalcycles == 4);
    }
}

TEST_CASE("Instruction BEQ", "[Processor][BEQ]") {
    CpuTestFixture sys;
    Byte opcode = 0xF0;

    SECTION("Branch not taken") {
        sys.load_program({
            { 0x8000, {opcode, 0x20} }
        });
        sys.processor.registerSet(Mos6502::Register::Status, 0x00);
        sys.processor.step();
        auto state = sys.state();
        REQUIRE(state.pc == 0x8002);
        REQUIRE(state.totalcycles == 2);
    }

    SECTION("Branch taken (no page crossing)") {
        sys.load_program({
            { 0x8000, {opcode, 0x20} }
        });
        sys.processor.registerSet(Mos6502::Register::Status, 0x00 | FLAG_ZERO);
        sys.processor.step();
        auto state = sys.state();
        REQUIRE(state.pc == 0x8022);
        REQUIRE(state.totalcycles == 3);
    }

    SECTION("Branch taken (with page crossing)") {
        sys.load_program({
            { 0x80C0, {opcode, 0x40} }
        });
        sys.processor.registerSet(Mos6502::Register::Status, 0x00 | FLAG_ZERO);
        sys.processor.step();
        auto state = sys.state();
        REQUIRE(state.pc == 0x8102);
        REQUIRE(state.totalcycles == 4);
    }
}

TEST_CASE("Instruction BMI", "[Processor][BMI]") {
    CpuTestFixture sys;
    Byte opcode = 0x30;

    SECTION("Branch not taken") {
        sys.load_program({
            { 0x8000, {opcode, 0x20} }
        });
        sys.processor.registerSet(Mos6502::Register::Status, 0x00);
        sys.processor.step();
        auto state = sys.state();
        REQUIRE(state.pc == 0x8002);
        REQUIRE(state.totalcycles == 2);
    }

    SECTION("Branch taken (no page crossing)") {
        sys.load_program({
            { 0x8000, {opcode, 0x20} }
        });
        sys.processor.registerSet(Mos6502::Register::Status, 0x00 | FLAG_NEGATIVE);
        sys.processor.step();
        auto state = sys.state();
        REQUIRE(state.pc == 0x8022);
        REQUIRE(state.totalcycles == 3);
    }

    SECTION("Branch taken (with page crossing)") {
        sys.load_program({
            { 0x80C0, {opcode, 0x40} }
        });
        sys.processor.registerSet(Mos6502::Register::Status, 0x00 | FLAG_NEGATIVE);
        sys.processor.step();
        auto state = sys.state();
        REQUIRE(state.pc == 0x8102);
        REQUIRE(state.totalcycles == 4);
    }
}

TEST_CASE("Instruction BNE", "[Processor][BNE]") {
    CpuTestFixture sys;
    Byte opcode = 0xD0;

    SECTION("Branch not taken") {
        sys.load_program({
            { 0x8000, {opcode, 0x20} }
        });
        sys.processor.registerSet(Mos6502::Register::Status, 0x00 | FLAG_ZERO);
        sys.processor.step();
        auto state = sys.state();
        REQUIRE(state.pc == 0x8002);
        REQUIRE(state.totalcycles == 2);
    }

    SECTION("Branch taken (no page crossing)") {
        sys.load_program({
            { 0x8000, {opcode, 0x20} }
        });
        sys.processor.registerSet(Mos6502::Register::Status, 0x00);
        sys.processor.step();
        auto state = sys.state();
        REQUIRE(state.pc == 0x8022);
        REQUIRE(state.totalcycles == 3);
    }

    SECTION("Branch taken (with page crossing)") {
        sys.load_program({
            { 0x80C0, {opcode, 0x40} }
        });
        sys.processor.registerSet(Mos6502::Register::Status, 0x00);
        sys.processor.step();
        auto state = sys.state();
        REQUIRE(state.pc == 0x8102);
        REQUIRE(state.totalcycles == 4);
    }
}

TEST_CASE("Instruction BPL", "[Processor][BPL]") {
    CpuTestFixture sys;
    Byte opcode = 0x10;

    SECTION("Branch not taken") {
        sys.load_program({
            { 0x8000, {opcode, 0x20} }
        });
        sys.processor.registerSet(Mos6502::Register::Status, 0x00 | FLAG_NEGATIVE);
        sys.processor.step();
        auto state = sys.state();
        REQUIRE(state.pc == 0x8002);
        REQUIRE(state.totalcycles == 2);
    }

    SECTION("Branch taken (no page crossing)") {
        sys.load_program({
            { 0x8000, {opcode, 0x20} }
        });
        sys.processor.registerSet(Mos6502::Register::Status, 0x00);
        sys.processor.step();
        auto state = sys.state();
        REQUIRE(state.pc == 0x8022);
        REQUIRE(state.totalcycles == 3);
    }

    SECTION("Branch taken (with page crossing)") {
        sys.load_program({
            { 0x80C0, {opcode, 0x40} }
        });
        sys.processor.registerSet(Mos6502::Register::Status, 0x00);
        sys.processor.step();
        auto state = sys.state();
        REQUIRE(state.pc == 0x8102);
        REQUIRE(state.totalcycles == 4);
    }
}

TEST_CASE("Instruction BVC", "[Processor][BVC]") {
    CpuTestFixture sys;
    Byte opcode = 0x50;

    SECTION("Branch not taken") {
        sys.load_program({
            { 0x8000, {opcode, 0x20} }
        });
        sys.processor.registerSet(Mos6502::Register::Status, 0x00 | FLAG_OVERFLOW);
        sys.processor.step();
        auto state = sys.state();
        REQUIRE(state.pc == 0x8002);
        REQUIRE(state.totalcycles == 2);
    }

    SECTION("Branch taken (no page crossing)") {
        sys.load_program({
            { 0x8000, {opcode, 0x20} }
        });
        sys.processor.registerSet(Mos6502::Register::Status, 0x00);
        sys.processor.step();
        auto state = sys.state();
        REQUIRE(state.pc == 0x8022);
        REQUIRE(state.totalcycles == 3);
    }

    SECTION("Branch taken (with page crossing)") {
        sys.load_program({
            { 0x80C0, {opcode, 0x40} }
        });
        sys.processor.registerSet(Mos6502::Register::Status, 0x00);
        sys.processor.step();
        auto state = sys.state();
        REQUIRE(state.pc == 0x8102);
        REQUIRE(state.totalcycles == 4);
    }
}

TEST_CASE("Instruction BVS", "[Processor][BVS]") {
    CpuTestFixture sys;
    Byte opcode = 0x70;

    SECTION("Branch not taken") {
        sys.load_program({
            { 0x8000, {opcode, 0x20} }
        });
        sys.processor.registerSet(Mos6502::Register::Status, 0x00);
        sys.processor.step();
        auto state = sys.state();
        REQUIRE(state.pc == 0x8002);
        REQUIRE(state.totalcycles == 2);
    }

    SECTION("Branch taken (no page crossing)") {
        sys.load_program({
            { 0x8000, {opcode, 0x20} }
        });
        sys.processor.registerSet(Mos6502::Register::Status, 0x00 | FLAG_OVERFLOW);
        sys.processor.step();
        auto state = sys.state();
        REQUIRE(state.pc == 0x8022);
        REQUIRE(state.totalcycles == 3);
    }

    SECTION("Branch taken (with page crossing)") {
        sys.load_program({
            { 0x80C0, {opcode, 0x40} }
        });
        sys.processor.registerSet(Mos6502::Register::Status, 0x00 | FLAG_OVERFLOW);
        sys.processor.step();
        auto state = sys.state();
        REQUIRE(state.pc == 0x8102);
        REQUIRE(state.totalcycles == 4);
    }
}

/*************************************
 * Group: Flag Instructions          *
 * CLC, CLD, CLI, CLV, SEC, SED, SEI *
 *************************************/

TEST_CASE("Instruction CLC", "[Processor][CLC]") {
    CpuTestFixture sys;
    Byte opcode = 0x18;

    sys.load_program({
        { 0x8000, {opcode} }
    });
    sys.processor.registerSet(Mos6502::Register::Status, 0x00 | FLAG_CARRY);
    sys.processor.step();
    auto state = sys.state();

    REQUIRE(state.pc == 0x8001);
    REQUIRE(state.totalcycles == 2);
    REQUIRE_FALSE(flag_status(FLAG_CARRY, state.status));
}

TEST_CASE("Instruction CLD", "[Processor][CLD]") {
    CpuTestFixture sys;
    Byte opcode = 0xD8;

    sys.load_program({
        { 0x8000, {opcode} }
    });
    sys.processor.registerSet(Mos6502::Register::Status, 0x00 | FLAG_DECIMAL);
    sys.processor.step();
    auto state = sys.state();

    REQUIRE(state.pc == 0x8001);
    REQUIRE(state.totalcycles == 2);
    REQUIRE_FALSE(flag_status(FLAG_DECIMAL, state.status));
}

TEST_CASE("Instruction CLI", "[Processor][CLI]") {
    CpuTestFixture sys;
    Byte opcode = 0x58;

    sys.load_program({
        { 0x8000, {opcode} }
    });
    sys.processor.registerSet(Mos6502::Register::Status, 0x00 | FLAG_INTERRUPT_DISABLE);
    sys.processor.step();
    auto state = sys.state();

    REQUIRE(state.pc == 0x8001);
    REQUIRE(state.totalcycles == 2);
    REQUIRE_FALSE(flag_status(FLAG_INTERRUPT_DISABLE, state.status));
}

TEST_CASE("Instruction CLV", "[Processor][CLV]") {
    CpuTestFixture sys;
    Byte opcode = 0xB8;

    sys.load_program({
        { 0x8000, {opcode} }
    });
    sys.processor.registerSet(Mos6502::Register::Status, 0x00 | FLAG_OVERFLOW);
    sys.processor.step();
    auto state = sys.state();

    REQUIRE(state.pc == 0x8001);
    REQUIRE(state.totalcycles == 2);
    REQUIRE_FALSE(flag_status(FLAG_OVERFLOW, state.status));
}

TEST_CASE("Instruction SEC", "[Processor][SEC]") {
    CpuTestFixture sys;
    Byte opcode = 0x38;

    sys.load_program({
        { 0x8000, {opcode} }
    });
    sys.processor.registerSet(Mos6502::Register::Status, 0x00);
    sys.processor.step();
    auto state = sys.state();

    REQUIRE(state.pc == 0x8001);
    REQUIRE(state.totalcycles == 2);
    REQUIRE(flag_status(FLAG_CARRY, state.status));
}

TEST_CASE("Instruction SED", "[Processor][SED]") {
    CpuTestFixture sys;
    Byte opcode = 0xF8;

    sys.load_program({
        { 0x8000, {opcode} }
    });
    sys.processor.registerSet(Mos6502::Register::Status, 0x00);
    sys.processor.step();
    auto state = sys.state();

    REQUIRE(state.pc == 0x8001);
    REQUIRE(state.totalcycles == 2);
    REQUIRE(flag_status(FLAG_DECIMAL, state.status));
}

TEST_CASE("Instruction SEI", "[Processor][SEI]") {
    CpuTestFixture sys;
    Byte opcode = 0x78;

    sys.load_program({
        { 0x8000, {opcode} }
    });
    sys.processor.registerSet(Mos6502::Register::Status, 0x00);
    sys.processor.step();
    auto state = sys.state();

    REQUIRE(state.pc == 0x8001);
    REQUIRE(state.totalcycles == 2);
    REQUIRE(flag_status(FLAG_INTERRUPT_DISABLE, state.status));
}

/********************************
 * Group: Transfer Instructions *
 * LDA, LDX, LDY, STA, STX, STY *
 * TAX, TAY, TSX, TXA, TXS, TYA *
 ********************************/

// TODO LDA
// TODO LDX
// TODO LDY
// TODO STA
// TODO STX
// TODO STY
// TODO TAX
// TODO TAY
// TODO TSX
// TODO TXA
// TODO TXS
// TODO TYA,

/*****************************
 * Group: Stack Instructions *
 * PHA, PHP, PLA, PLP        *
 *****************************/

// TODO PHA
// TODO PHP
// TODO PLA
// TODO PLP

/***********************************
 * Group: INC and DEC Instructions *
 * DEC, DEX, DEY, INC, INX, INY    *
 ***********************************/

// TODO DEC
// TODO DEX
// TODO DEY
// TODO INC
// TODO INX
// TODO INY

/**********************************
 * Group: Arithmetic Instructions *
 * ADC, SBC                       *
 **********************************/

TEST_CASE("Instruction ADC", "[Processor][ADC]") {
    // TODO Implement
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

TEST_CASE("Instruction SBC", "[Processor][SBC]") {
    // TODO Implement
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

/*****************************************
 * Group: Logical Operation Instructions *
 * AND, EOR, ORA                         *
 *****************************************/

// TODO AND
// TODO EOR
// TODO ORA

/****************************************
 * Group: Shift and Rotate Instructions *
 * ASL, LSR, ROL, ROR                   *
 ****************************************/

// TODO ASL
// TODO LSR
// TODO ROL
// TODO ROR

/**********************************
 * Group: Comparison Instructions *
 * BIT, CMP, CPX, CPY             *
 **********************************/

// TODO BIT
// TODO CMP
// TODO CPX
// TODO CPY

/******************************************
 * Group: JMP and Subroutine Instructions *
 * JMP, JSR, RTS                          *
 ******************************************/

// TODO JMP
// TODO JSR
// TODO RTS

/*****************************
 * Group: Other Instructions *
 * BRK, NOP, RTI             *
 *****************************/

// TODO BRK
// TODO NOP
// TODO RTI
