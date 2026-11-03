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

TEST_CASE("Instruction LDA", "[Processor][LDA]") {
    CpuTestFixture sys;
    Byte operand = 0x45;

    SECTION("Addressing mode: Immediate") {
        Byte opcode = 0xA9;
        sys.load_program({
            { 0x8000, {opcode, operand} }
        });
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8002);
        REQUIRE(state.totalcycles == 2);
        REQUIRE(state.a == operand);
    }

    SECTION("Addressing mode: Zero Page") {
        Byte opcode = 0xA5;
        sys.load_program({
            { 0x8000, {opcode, 0x20} },
            { 0x0020, {operand} }
        });
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8002);
        REQUIRE(state.totalcycles == 3);
        REQUIRE(state.a == operand);
    }

    SECTION("Addressing mode: Zero Page, X") {
        Byte opcode = 0xB5;
        sys.load_program({
            { 0x8000, {opcode, 0x20} },
            { 0x0025, {operand} }
        });
        sys.processor.registerSet(Mos6502::Register::X, 0x05);
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8002);
        REQUIRE(state.totalcycles == 4);
        REQUIRE(state.a == operand);
    }

    SECTION("Addressing mode: Absolute") {
        Byte opcode = 0xAD;
        sys.load_program({
            { 0x8000, {opcode, 0x00, 0x20} },
            { 0x2000, {operand} }
        });
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8003);
        REQUIRE(state.totalcycles == 4);
        REQUIRE(state.a == operand);
    }

    SECTION("Addressing mode: Absolute, X") {
        Byte opcode = 0xBD;
        sys.load_program({
            { 0x8000, {opcode, 0x00, 0x20} },
            { 0x2005, {operand} }
        });
        sys.processor.registerSet(Mos6502::Register::X, 0x05);
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8003);
        REQUIRE(state.totalcycles == 4);
        REQUIRE(state.a == operand);
    }

    SECTION("Addressing mode: Absolute, X (page cross)") {
        Byte opcode = 0xBD;
        sys.load_program({
            { 0x8000, {opcode, 0xFF, 0x20} },
            { 0x2104, {operand} }
        });
        sys.processor.registerSet(Mos6502::Register::X, 0x05);
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8003);
        REQUIRE(state.totalcycles == 5);
        REQUIRE(state.a == operand);
    }

    SECTION("Addressing mode: Absolute, Y") {
        Byte opcode = 0xB9;
        sys.load_program({
            { 0x8000, {opcode, 0x00, 0x20} },
            { 0x2005, {operand} }
        });
        sys.processor.registerSet(Mos6502::Register::Y, 0x05);
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8003);
        REQUIRE(state.totalcycles == 4);
        REQUIRE(state.a == operand);
    }

    SECTION("Addressing mode: Absolute, Y (page cross)") {
        Byte opcode = 0xB9;
        sys.load_program({
            { 0x8000, {opcode, 0xFF, 0x20} },
            { 0x2104, {operand} }
        });
        sys.processor.registerSet(Mos6502::Register::Y, 0x05);
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8003);
        REQUIRE(state.totalcycles == 5);
        REQUIRE(state.a == operand);
    }

    SECTION("Addressing mode: Indirect, X") {
        Byte opcode = 0xA1;
        sys.load_program({
            { 0x8000, {opcode, 0x20} },
            { 0x0025, {0x00, 0x20} },
            { 0x2000, {operand} }
        });
        sys.processor.registerSet(Mos6502::Register::X, 0x05);
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8002);
        REQUIRE(state.totalcycles == 6);
        REQUIRE(state.a == operand);
    }

    SECTION("Addressing mode: Indirect, Y") {
        Byte opcode = 0xB1;
        sys.load_program({
            { 0x8000, {opcode, 0x20} },
            { 0x0020, {0x00, 0x20} },
            { 0x2005, {operand} }
        });
        sys.processor.registerSet(Mos6502::Register::Y, 0x05);
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8002);
        REQUIRE(state.totalcycles == 5);
        REQUIRE(state.a == operand);
    }

    SECTION("Addressing mode: Indirect, Y (page cross)") {
        Byte opcode = 0xB1;
        sys.load_program({
            { 0x8000, {opcode, 0x20} },
            { 0x0020, {0xFF, 0x20} },
            { 0x2104, {operand} }
        });
        sys.processor.registerSet(Mos6502::Register::Y, 0x05);
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8002);
        REQUIRE(state.totalcycles == 6);
        REQUIRE(state.a == operand);
    }

    SECTION("Flag behavior: Negative") {
        Byte opcode = 0xA9; // Immediate
        sys.load_program({
            { 0x8000, {opcode, 0x81} }
        });
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8002);
        REQUIRE(state.totalcycles == 2);
        REQUIRE(state.a == 0x81);
        REQUIRE(flag_status(FLAG_NEGATIVE, state.status));
    }

    SECTION("Flag behavior: Zero") {
        Byte opcode = 0xA9; // Immediate
        sys.load_program({
            { 0x8000, {opcode, 0x00} }
        });
        sys.processor.registerSet(Mos6502::Register::A, 0x20); // Set to a garbage non-zero value
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8002);
        REQUIRE(state.totalcycles == 2);
        REQUIRE(state.a == 0x00);
        REQUIRE(flag_status(FLAG_ZERO, state.status));
    }

}

TEST_CASE("Instruction LDX", "[Processor][LDX]") {
    CpuTestFixture sys;
    Byte operand = 0x45;

    SECTION("Addressing mode: Immediate") {
        Byte opcode = 0xA2;
        sys.load_program({
            { 0x8000, {opcode, operand} }
        });
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8002);
        REQUIRE(state.totalcycles == 2);
        REQUIRE(state.x == operand);
    }

    SECTION("Addressing mode: Zero Page") {
        Byte opcode = 0xA6;
        sys.load_program({
            { 0x8000, {opcode, 0x20} },
            { 0x0020, {operand} }
        });
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8002);
        REQUIRE(state.totalcycles == 3);
        REQUIRE(state.x == operand);
    }

    SECTION("Addressing mode: Zero Page, Y") {
        Byte opcode = 0xB6;
        sys.load_program({
            { 0x8000, {opcode, 0x20} },
            { 0x0025, {operand} }
        });
        sys.processor.registerSet(Mos6502::Register::Y, 0x05);
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8002);
        REQUIRE(state.totalcycles == 4);
        REQUIRE(state.x == operand);
    }

    SECTION("Addressing mode: Absolute") {
        Byte opcode = 0xAE;
        sys.load_program({
            { 0x8000, {opcode, 0x00, 0x20} },
            { 0x2000, {operand} }
        });
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8003);
        REQUIRE(state.totalcycles == 4);
        REQUIRE(state.x == operand);
    }

    SECTION("Addressing mode: Absolute, Y") {
        Byte opcode = 0xBE;
        sys.load_program({
            { 0x8000, {opcode, 0x00, 0x20} },
            { 0x2005, {operand} }
        });
        sys.processor.registerSet(Mos6502::Register::Y, 0x05);
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8003);
        REQUIRE(state.totalcycles == 4);
        REQUIRE(state.x == operand);
    }

    SECTION("Addressing mode: Absolute, Y (page cross)") {
        Byte opcode = 0xBE;
        sys.load_program({
            { 0x8000, {opcode, 0xFF, 0x20} },
            { 0x2104, {operand} }
        });
        sys.processor.registerSet(Mos6502::Register::Y, 0x05);
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8003);
        REQUIRE(state.totalcycles == 5);
        REQUIRE(state.x == operand);
    }

    SECTION("Flag behavior: Negative") {
        Byte opcode = 0xA2; // Immediate
        sys.load_program({
            { 0x8000, {opcode, 0x81} }
        });
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8002);
        REQUIRE(state.totalcycles == 2);
        REQUIRE(state.x == 0x81);
        REQUIRE(flag_status(FLAG_NEGATIVE, state.status));
    }

    SECTION("Flag behavior: Zero") {
        Byte opcode = 0xA2; // Immediate
        sys.load_program({
            { 0x8000, {opcode, 0x00} }
        });
        sys.processor.registerSet(Mos6502::Register::X, 0x20); // Set to a garbage non-zero value
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8002);
        REQUIRE(state.totalcycles == 2);
        REQUIRE(state.x == 0x00);
        REQUIRE(flag_status(FLAG_ZERO, state.status));
    }

}

TEST_CASE("Instruction LDY", "[Processor][LDY]") {
    CpuTestFixture sys;
    Byte operand = 0x45;

    SECTION("Addressing mode: Immediate") {
        Byte opcode = 0xA0;
        sys.load_program({
            { 0x8000, {opcode, operand} }
        });
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8002);
        REQUIRE(state.totalcycles == 2);
        REQUIRE(state.y == operand);
    }

    SECTION("Addressing mode: Zero Page") {
        Byte opcode = 0xA4;
        sys.load_program({
            { 0x8000, {opcode, 0x20} },
            { 0x0020, {operand} }
        });
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8002);
        REQUIRE(state.totalcycles == 3);
        REQUIRE(state.y == operand);
    }

    SECTION("Addressing mode: Zero Page, X") {
        Byte opcode = 0xB4;
        sys.load_program({
            { 0x8000, {opcode, 0x20} },
            { 0x0025, {operand} }
        });
        sys.processor.registerSet(Mos6502::Register::X, 0x05);
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8002);
        REQUIRE(state.totalcycles == 4);
        REQUIRE(state.y == operand);
    }

    SECTION("Addressing mode: Absolute") {
        Byte opcode = 0xAC;
        sys.load_program({
            { 0x8000, {opcode, 0x00, 0x20} },
            { 0x2000, {operand} }
        });
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8003);
        REQUIRE(state.totalcycles == 4);
        REQUIRE(state.y == operand);
    }

    SECTION("Addressing mode: Absolute, X") {
        Byte opcode = 0xBC;
        sys.load_program({
            { 0x8000, {opcode, 0x00, 0x20} },
            { 0x2005, {operand} }
        });
        sys.processor.registerSet(Mos6502::Register::X, 0x05);
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8003);
        REQUIRE(state.totalcycles == 4);
        REQUIRE(state.y == operand);
    }

    SECTION("Addressing mode: Absolute, X (page cross)") {
        Byte opcode = 0xBC;
        sys.load_program({
            { 0x8000, {opcode, 0xFF, 0x20} },
            { 0x2104, {operand} }
        });
        sys.processor.registerSet(Mos6502::Register::X, 0x05);
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8003);
        REQUIRE(state.totalcycles == 5);
        REQUIRE(state.y == operand);
    }

    SECTION("Flag behavior: Negative") {
        Byte opcode = 0xA0; // Immediate
        sys.load_program({
            { 0x8000, {opcode, 0x81} }
        });
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8002);
        REQUIRE(state.totalcycles == 2);
        REQUIRE(state.y == 0x81);
        REQUIRE(flag_status(FLAG_NEGATIVE, state.status));
    }

    SECTION("Flag behavior: Zero") {
        Byte opcode = 0xA0; // Immediate
        sys.load_program({
            { 0x8000, {opcode, 0x00} }
        });
        sys.processor.registerSet(Mos6502::Register::X, 0x20); // Set to a garbage non-zero value
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8002);
        REQUIRE(state.totalcycles == 2);
        REQUIRE(state.y == 0x00);
        REQUIRE(flag_status(FLAG_ZERO, state.status));
    }

}

TEST_CASE("Instruction STA", "[Processor][STA]") {
    CpuTestFixture sys;
    Byte regVal = 0x20;

    SECTION("Addressing mode: Zero Page") {
        Byte opcode = 0x85;
        sys.load_program({
            { 0x8000, {opcode, 0x80} }
        });
        sys.processor.registerSet(Mos6502::Register::A, regVal);
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8002);
        REQUIRE(state.totalcycles == 3);
        REQUIRE(sys.memory[0x80] == regVal);
    }

    SECTION("Addressing mode: Zero Page, X") {
        Byte opcode = 0x95;
        sys.load_program({
            { 0x8000, {opcode, 0x80} }
        });
        sys.processor.registerSet(Mos6502::Register::A, regVal);
        sys.processor.registerSet(Mos6502::Register::X, 0x05);
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8002);
        REQUIRE(state.totalcycles == 4);
        REQUIRE(sys.memory[0x85] == regVal);
    }

    SECTION("Addressing mode: Absolute") {
        Byte opcode = 0x8D;
        sys.load_program({
            { 0x8000, {opcode, 0x00, 0x20} }
        });
        sys.processor.registerSet(Mos6502::Register::A, regVal);
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8003);
        REQUIRE(state.totalcycles == 4);
        REQUIRE(sys.memory[0x2000] == regVal);
    }

    SECTION("Addressing mode: Absolute, X") {
        Byte opcode = 0x9D;
        sys.load_program({
            { 0x8000, {opcode, 0x00, 0x20} }
        });
        sys.processor.registerSet(Mos6502::Register::A, regVal);
        sys.processor.registerSet(Mos6502::Register::X, 0x05);
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8003);
        REQUIRE(state.totalcycles == 5);
        REQUIRE(sys.memory[0x2005] == regVal);
    }

    SECTION("Addressing mode: Absolute, X (page cross)") {
        Byte opcode = 0x9D;
        sys.load_program({
            { 0x8000, {opcode, 0xFF, 0x20} }
        });
        sys.processor.registerSet(Mos6502::Register::A, regVal);
        sys.processor.registerSet(Mos6502::Register::X, 0x05);
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8003);
        REQUIRE(state.totalcycles == 6);
        REQUIRE(sys.memory[0x2104] == regVal);
    }

    SECTION("Addressing mode: Absolute, Y") {
        Byte opcode = 0x99;
        sys.load_program({
            { 0x8000, {opcode, 0x00, 0x20} }
        });
        sys.processor.registerSet(Mos6502::Register::A, regVal);
        sys.processor.registerSet(Mos6502::Register::Y, 0x05);
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8003);
        REQUIRE(state.totalcycles == 5);
        REQUIRE(sys.memory[0x2005] == regVal);
    }

    SECTION("Addressing mode: Absolute, Y (page cross)") {
        Byte opcode = 0x99;
        sys.load_program({
            { 0x8000, {opcode, 0xFF, 0x20} }
        });
        sys.processor.registerSet(Mos6502::Register::A, regVal);
        sys.processor.registerSet(Mos6502::Register::Y, 0x05);
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8003);
        REQUIRE(state.totalcycles == 6);
        REQUIRE(sys.memory[0x2104] == regVal);
    }

    SECTION("Addressing mode: Indirect, X") {
        Byte opcode = 0x81;
        sys.load_program({
            { 0x8000, {opcode, 0x20} },
            { 0x0025, {0x00, 0x20} }
        });
        sys.processor.registerSet(Mos6502::Register::A, regVal);
        sys.processor.registerSet(Mos6502::Register::X, 0x05);
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8002);
        REQUIRE(state.totalcycles == 6);
        REQUIRE(sys.memory[0x2000] == regVal);
    }

    SECTION("Addressing mode: Indirect, Y") {
        Byte opcode = 0x91;
        sys.load_program({
            { 0x8000, {opcode, 0x20} },
            { 0x0020, {0x00, 0x20} }
        });
        sys.processor.registerSet(Mos6502::Register::A, regVal);
        sys.processor.registerSet(Mos6502::Register::Y, 0x05);
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8002);
        REQUIRE(state.totalcycles == 6);
        REQUIRE(sys.memory[0x2005] == regVal);
    }

    SECTION("Addressing mode: Indirect, Y (page cross)") {
        Byte opcode = 0x91;
        sys.load_program({
            { 0x8000, {opcode, 0x20} },
            { 0x0020, {0xFF, 0x20} }
        });
        sys.processor.registerSet(Mos6502::Register::A, regVal);
        sys.processor.registerSet(Mos6502::Register::Y, 0x05);
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8002);
        REQUIRE(state.totalcycles == 7);
        REQUIRE(sys.memory[0x2104] == regVal);
    }
}

TEST_CASE("Instruction STX", "[Processor][STX]") {
    CpuTestFixture sys;
    Byte regVal = 0x20;

    SECTION("Addressing mode: Zero Page") {
        Byte opcode = 0x86;
        sys.load_program({
            { 0x8000, {opcode, 0x80} }
        });
        sys.processor.registerSet(Mos6502::Register::X, regVal);
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8002);
        REQUIRE(state.totalcycles == 3);
        REQUIRE(sys.memory[0x80] == regVal);
    }

    SECTION("Addressing mode: Zero Page, Y") {
        Byte opcode = 0x96;
        sys.load_program({
            { 0x8000, {opcode, 0x80} }
        });
        sys.processor.registerSet(Mos6502::Register::X, regVal);
        sys.processor.registerSet(Mos6502::Register::Y, 0x05);
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8002);
        REQUIRE(state.totalcycles == 4);
        REQUIRE(sys.memory[0x85] == regVal);
    }

    SECTION("Addressing mode: Absolute") {
        Byte opcode = 0x8E;
        sys.load_program({
            { 0x8000, {opcode, 0x00, 0x20} }
        });
        sys.processor.registerSet(Mos6502::Register::X, regVal);
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8003);
        REQUIRE(state.totalcycles == 4);
        REQUIRE(sys.memory[0x2000] == regVal);
    }

}

TEST_CASE("Instruction STY", "[Processor][STY]") {
    CpuTestFixture sys;
    Byte regVal = 0x20;

    SECTION("Addressing mode: Zero Page") {
        Byte opcode = 0x84;
        sys.load_program({
            { 0x8000, {opcode, 0x80} }
        });
        sys.processor.registerSet(Mos6502::Register::Y, regVal);
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8002);
        REQUIRE(state.totalcycles == 3);
        REQUIRE(sys.memory[0x80] == regVal);
    }

    SECTION("Addressing mode: Zero Page, X") {
        Byte opcode = 0x94;
        sys.load_program({
            { 0x8000, {opcode, 0x80} }
        });
        sys.processor.registerSet(Mos6502::Register::Y, regVal);
        sys.processor.registerSet(Mos6502::Register::X, 0x05);
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8002);
        REQUIRE(state.totalcycles == 4);
        REQUIRE(sys.memory[0x85] == regVal);
    }

    SECTION("Addressing mode: Absolute") {
        Byte opcode = 0x8C;
        sys.load_program({
            { 0x8000, {opcode, 0x00, 0x20} }
        });
        sys.processor.registerSet(Mos6502::Register::Y, regVal);
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8003);
        REQUIRE(state.totalcycles == 4);
        REQUIRE(sys.memory[0x2000] == regVal);
    }

}

TEST_CASE("Instruction TAX", "[Processor][TAX]") {
    CpuTestFixture sys;
    Byte opcode = 0xAA;
    sys.load_program({
        { 0x8000, {opcode} }
    });

    SECTION("General behavior (with N and Z clear)") {
        sys.processor.registerSet(Mos6502::Register::A, 0x20);
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8001);
        REQUIRE(state.totalcycles == 2);
        REQUIRE(state.x == 0x20);
        REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
        REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
    }

    SECTION("Flag behavior: Negative (set)") {
        sys.processor.registerSet(Mos6502::Register::A, 0x81);
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8001);
        REQUIRE(state.totalcycles == 2);
        REQUIRE(state.x == 0x81);
        REQUIRE(flag_status(FLAG_NEGATIVE, state.status));
    }

    SECTION("Flag behavior: Zero (set)") {
        sys.processor.registerSet(Mos6502::Register::A, 0x00);
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8001);
        REQUIRE(state.totalcycles == 2);
        REQUIRE(state.x == 0x00);
        REQUIRE(flag_status(FLAG_ZERO, state.status));
    }

}

TEST_CASE("Instruction TAY", "[Processor][TAY]") {
    CpuTestFixture sys;
    Byte opcode = 0xA8;
    sys.load_program({
        { 0x8000, {opcode} }
    });

    SECTION("General behavior (with N and Z clear)") {
        sys.processor.registerSet(Mos6502::Register::A, 0x20);
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8001);
        REQUIRE(state.totalcycles == 2);
        REQUIRE(state.y == 0x20);
        REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
        REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
    }

    SECTION("Flag behavior: Negative (set)") {
        sys.processor.registerSet(Mos6502::Register::A, 0x81);
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8001);
        REQUIRE(state.totalcycles == 2);
        REQUIRE(state.y == 0x81);
        REQUIRE(flag_status(FLAG_NEGATIVE, state.status));
    }

    SECTION("Flag behavior: Zero (set)") {
        sys.processor.registerSet(Mos6502::Register::A, 0x00);
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8001);
        REQUIRE(state.totalcycles == 2);
        REQUIRE(state.y == 0x00);
        REQUIRE(flag_status(FLAG_ZERO, state.status));
    }

}

TEST_CASE("Instruction TSX", "[Processor][TSX]") {
    CpuTestFixture sys;
    Byte opcode = 0xBA;
    sys.load_program({
        { 0x8000, {opcode} }
    });

    SECTION("General behavior (with N and Z clear)") {
        sys.processor.registerSet(Mos6502::Register::StackPointer, 0x20);
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8001);
        REQUIRE(state.totalcycles == 2);
        REQUIRE(state.x == 0x20);
        REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
        REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
    }

    SECTION("Flag behavior: Negative (set)") {
        sys.processor.registerSet(Mos6502::Register::StackPointer, 0x81);
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8001);
        REQUIRE(state.totalcycles == 2);
        REQUIRE(state.x == 0x81);
        REQUIRE(flag_status(FLAG_NEGATIVE, state.status));
        REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
    }

    SECTION("Flag behavior: Zero (set)") {
        sys.processor.registerSet(Mos6502::Register::X, 0xFF);
        sys.processor.registerSet(Mos6502::Register::StackPointer, 0x00);
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8001);
        REQUIRE(state.totalcycles == 2);
        REQUIRE(state.x == 0x00);
        REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
        REQUIRE(flag_status(FLAG_ZERO, state.status));
    }

}

TEST_CASE("Instruction TXA", "[Processor][TXA]") {
    CpuTestFixture sys;
    Byte opcode = 0x8A;
    sys.load_program({
        { 0x8000, {opcode} }
    });

    SECTION("General behavior (with N and Z clear)") {
        sys.processor.registerSet(Mos6502::Register::X, 0x20);
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8001);
        REQUIRE(state.totalcycles == 2);
        REQUIRE(state.a == 0x20);
        REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
        REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
    }

    SECTION("Flag behavior: Negative (set)") {
        sys.processor.registerSet(Mos6502::Register::X, 0x81);
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8001);
        REQUIRE(state.totalcycles == 2);
        REQUIRE(state.a == 0x81);
        REQUIRE(flag_status(FLAG_NEGATIVE, state.status));
        REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
    }

    SECTION("Flag behavior: Zero (set)") {
        sys.processor.registerSet(Mos6502::Register::A, 0xFF);
        sys.processor.registerSet(Mos6502::Register::X, 0x00);
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8001);
        REQUIRE(state.totalcycles == 2);
        REQUIRE(state.a == 0x00);
        REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
        REQUIRE(flag_status(FLAG_ZERO, state.status));
    }

}

TEST_CASE("Instruction TXS", "[Processor][TXS]") {
    CpuTestFixture sys;
    Byte opcode = 0x9A;
    sys.load_program({
        { 0x8000, {opcode} }
    });
    sys.processor.registerSet(Mos6502::Register::X, 0x20);
    sys.processor.step();
    auto state = sys.state();

    REQUIRE(state.pc == 0x8001);
    REQUIRE(state.totalcycles == 2);
    REQUIRE(state.stackptr == 0x20);
}

TEST_CASE("Instruction TYA", "[Processor][TYA]") {
    CpuTestFixture sys;
    Byte opcode = 0x98;
    sys.load_program({
        { 0x8000, {opcode} }
    });

    SECTION("General behavior (with N and Z clear)") {
        sys.processor.registerSet(Mos6502::Register::Y, 0x20);
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8001);
        REQUIRE(state.totalcycles == 2);
        REQUIRE(state.a == 0x20);
        REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
        REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
    }

    SECTION("Flag behavior: Negative (set)") {
        sys.processor.registerSet(Mos6502::Register::Y, 0x81);
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8001);
        REQUIRE(state.totalcycles == 2);
        REQUIRE(state.a == 0x81);
        REQUIRE(flag_status(FLAG_NEGATIVE, state.status));
        REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
    }

    SECTION("Flag behavior: Zero (set)") {
        sys.processor.registerSet(Mos6502::Register::A, 0xFF);
        sys.processor.registerSet(Mos6502::Register::Y, 0x00);
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8001);
        REQUIRE(state.totalcycles == 2);
        REQUIRE(state.a == 0x00);
        REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
        REQUIRE(flag_status(FLAG_ZERO, state.status));
    }

}

/*****************************
 * Group: Stack Instructions *
 * PHA, PHP, PLA, PLP        *
 *****************************/

TEST_CASE("Instruction PHA", "[Processor][PHA]") {
    CpuTestFixture sys;
    Byte opcode = 0x48;
    sys.load_program({
        { 0x8000, {opcode} }
    });
    sys.processor.registerSet(Mos6502::Register::A, 0x20);
    sys.processor.step();
    auto state = sys.state();

    REQUIRE(state.pc == 0x8001);
    REQUIRE(state.totalcycles == 3);
    REQUIRE(state.stackptr == 0xFE);
    REQUIRE(sys.memory[0x01FF] == 0x20);
}

TEST_CASE("Instruction PHP", "[Processor][PHP]") {
    CpuTestFixture sys;
    Byte opcode = 0x08;
    sys.load_program({
        { 0x8000, {opcode} }
    });
    sys.processor.registerSet(Mos6502::Register::Status, 0b11001111);
    sys.processor.step();
    auto state = sys.state();

    REQUIRE(state.pc == 0x8001);
    REQUIRE(state.totalcycles == 3);
    REQUIRE(state.stackptr == 0xFE);
    REQUIRE(sys.memory[0x01FF] == 0xFF);
}

TEST_CASE("Instruction PLA", "[Processor][PLA]") {
    CpuTestFixture sys;
    Byte opcode = 0x68;
    sys.load_program({
        { 0x8000, {opcode} }
    });
    sys.processor.registerSet(Mos6502::Register::StackPointer, 0xFE);

    SECTION("General behavior (N and Z clear)") {
        sys.memory[0x01FF] = 0x20;
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8001);
        REQUIRE(state.totalcycles == 4);
        REQUIRE(state.stackptr == 0xFF);
        REQUIRE(state.a == 0x20);
        REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
        REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
    }

    SECTION("Flag behavior: Negative (set)") {
        sys.memory[0x01FF] = 0x81;
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8001);
        REQUIRE(state.totalcycles == 4);
        REQUIRE(state.stackptr == 0xFF);
        REQUIRE(state.a == 0x81);
        REQUIRE(flag_status(state.status, FLAG_NEGATIVE));
        REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
    }

    SECTION("Flag behavior: Zero (set)") {
        sys.processor.registerSet(Mos6502::Register::A, 0x20);
        sys.memory[0x01FF] = 0x00;
        sys.processor.step();
        auto state = sys.state();

        REQUIRE(state.pc == 0x8001);
        REQUIRE(state.totalcycles == 4);
        REQUIRE(state.stackptr == 0xFF);
        REQUIRE(state.a == 0x00);
        REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
        REQUIRE(flag_status(state.status, FLAG_ZERO));
    }

}

TEST_CASE("Instruction PLP", "[Processor][PLP]") {
    CpuTestFixture sys;
    Byte opcode = 0x28;
    sys.load_program({
        { 0x8000, {opcode} }
    });
    sys.processor.registerSet(Mos6502::Register::StackPointer, 0xFE);
    sys.memory[0x01FF] = 0b11111111;
    sys.processor.step();
    auto state = sys.state();

    REQUIRE(state.pc == 0x8001);
    REQUIRE(state.totalcycles == 4);
    REQUIRE(state.stackptr == 0xFF);
    REQUIRE(state.status == 0b11101111);
    REQUIRE(flag_status(state.status, FLAG_NEGATIVE));
    REQUIRE(flag_status(state.status, FLAG_ZERO));
    REQUIRE(flag_status(state.status, FLAG_UNUSED));
    REQUIRE_FALSE(flag_status(state.status, FLAG_BREAK));
    REQUIRE(flag_status(state.status, FLAG_CARRY));
    REQUIRE(flag_status(state.status, FLAG_INTERRUPT_DISABLE));
    REQUIRE(flag_status(state.status, FLAG_DECIMAL));
    REQUIRE(flag_status(state.status, FLAG_OVERFLOW));
}

/***********************************
 * Group: INC and DEC Instructions *
 * DEC, DEX, DEY, INC, INX, INY    *
 ***********************************/

TEST_CASE("Instruction DEC", "[Processor][DEC]") {
    CpuTestFixture sys;
    Byte addr_hi = 0x20;
    Byte addr_offset = 0x05;
    Byte val = 0x40;

    SECTION("Addressing modes") {
        SECTION("Zero Page") {
            Byte opcode = 0xC6;
            sys.load_program({
                { 0x8000, {opcode, addr_hi} }
            });
            sys.memory[addr_hi] = val;
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(sys.memory[addr_hi] == val - 1);
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 5);
            REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
        }

        SECTION("Zero Page, X") {
            Byte opcode = 0xD6;
            sys.load_program({
                { 0x8000, {opcode, addr_hi} }
            });
            sys.processor.registerSet(Mos6502::Register::X, addr_offset);
            sys.memory[addr_hi + addr_offset] = val;
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(sys.memory[addr_hi + addr_offset] == val - 1);
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 6);
            REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
        }

        SECTION("Absolute") {
            Byte opcode = 0xCE;
            sys.load_program({
                { 0x8000, {opcode, 0x00, addr_hi} }
            });
            sys.memory[addr_hi << 8] = val;
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(sys.memory[addr_hi << 8] == val - 1);
            REQUIRE(state.pc == 0x8003);
            REQUIRE(state.totalcycles == 6);
            REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
        }

        SECTION("Absolute, X") {
            Byte opcode = 0xDE;
            sys.load_program({
                { 0x8000, {opcode, 0x00, addr_hi} }
            });
            sys.processor.registerSet(Mos6502::Register::X, addr_offset);
            sys.memory[(addr_hi << 8) + addr_offset] = val;
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(sys.memory[(addr_hi << 8) + addr_offset] == val - 1);
            REQUIRE(state.pc == 0x8003);
            REQUIRE(state.totalcycles == 7);
            REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
        }

        SECTION("Absolute, X (page cross)") {
            Byte opcode = 0xDE;
            sys.load_program({
                { 0x8000, {opcode, 0xFF, addr_hi} }
            });
            sys.processor.registerSet(Mos6502::Register::X, addr_offset);
            sys.memory[((addr_hi + 1) << 8) + (addr_offset - 1)] = val;
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(sys.memory[((addr_hi + 1) << 8) + (addr_offset - 1)] == val - 1);
            REQUIRE(state.pc == 0x8003);
            REQUIRE(state.totalcycles == 8);
            REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
        }
    }

    SECTION("Flag Behavior") {
        Byte opcode = 0xC6;
        sys.load_program({
            { 0x8000, {opcode, addr_hi} }
        });

        SECTION("Negative flag is set when result >= 0x80") {
            sys.memory[addr_hi] = 0x90;
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(sys.memory[addr_hi] == 0x8F);
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 5);
            REQUIRE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));

        }

        SECTION("Zero flag is set when result == 0") {
            sys.memory[addr_hi] = 0x01;
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(sys.memory[addr_hi] == 0x00);
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 5);
            REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE(flag_status(state.status, FLAG_ZERO));

        }
    }

    SECTION("Edge Cases") {
        Byte opcode = 0xC6;
        sys.load_program({
            { 0x8000, {opcode, addr_hi} }
        });

        SECTION("DEC 0x00 should result in 0xFF") {
            sys.memory[addr_hi] = 0x00;
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(sys.memory[addr_hi] == 0xFF);
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 5);
            REQUIRE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));

        }
    }
}

TEST_CASE("Instruction DEX", "[Processor][DEX]") {
    CpuTestFixture sys;
    Byte opcode = 0xCA;
    Byte val = 0x40;
    sys.load_program({
        { 0x8000, {opcode} }
    });

    SECTION("General Behavior") {
        sys.processor.registerSet(Mos6502::Register::X, val);
        sys.processor.step();
        auto state = sys.state();
        REQUIRE(state.x == val - 1);
        REQUIRE(state.pc == 0x8001);
        REQUIRE(state.totalcycles == 2);
        REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
        REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
    }

    SECTION("Flag Behavior") {
        SECTION("Negative flag is set when result >= 0x80") {
            sys.processor.registerSet(Mos6502::Register::X, 0x81);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.x == 0x80);
            REQUIRE(state.pc == 0x8001);
            REQUIRE(state.totalcycles == 2);
            REQUIRE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
        }

        SECTION("Zero flag is set when result == 0") {
            sys.processor.registerSet(Mos6502::Register::X, 0x01);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.x == 0x00);
            REQUIRE(state.pc == 0x8001);
            REQUIRE(state.totalcycles == 2);
            REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE(flag_status(state.status, FLAG_ZERO));
        }
    }

    SECTION("Edge Cases") {
        SECTION("DEX 0x00 should result in 0xFF") {
            sys.processor.registerSet(Mos6502::Register::X, 0x00);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.x == 0xFF);
            REQUIRE(state.pc == 0x8001);
            REQUIRE(state.totalcycles == 2);
            REQUIRE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
        }
    }
}

TEST_CASE("Instruction DEY", "[Processor][DEY]") {
    CpuTestFixture sys;
    Byte opcode = 0x88;
    Byte val = 0x40;
    sys.load_program({
        { 0x8000, {opcode} }
    });

    SECTION("General Behavior") {
        sys.processor.registerSet(Mos6502::Register::Y, val);
        sys.processor.step();
        auto state = sys.state();
        REQUIRE(state.y == val - 1);
        REQUIRE(state.pc == 0x8001);
        REQUIRE(state.totalcycles == 2);
        REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
        REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
    }

    SECTION("Flag Behavior") {
        SECTION("Negative flag is set when result >= 0x80") {
            sys.processor.registerSet(Mos6502::Register::Y, 0x81);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.y == 0x80);
            REQUIRE(state.pc == 0x8001);
            REQUIRE(state.totalcycles == 2);
            REQUIRE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
        }

        SECTION("Zero flag is set when result == 0") {
            sys.processor.registerSet(Mos6502::Register::Y, 0x01);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.y == 0x00);
            REQUIRE(state.pc == 0x8001);
            REQUIRE(state.totalcycles == 2);
            REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE(flag_status(state.status, FLAG_ZERO));
        }
    }

    SECTION("Edge Cases") {
        SECTION("DEY 0x00 should result in 0xFF") {
            sys.processor.registerSet(Mos6502::Register::Y, 0x00);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.y == 0xFF);
            REQUIRE(state.pc == 0x8001);
            REQUIRE(state.totalcycles == 2);
            REQUIRE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
        }
    }
}

TEST_CASE("Instruction INC", "[Processor][INC]") {
    CpuTestFixture sys;
    Byte addr_hi = 0x20;
    Byte addr_offset = 0x05;
    Byte val = 0x40;

    SECTION("Addressing modes") {
        SECTION("Zero Page") {
            Byte opcode = 0xE6;
            sys.load_program({
                { 0x8000, {opcode, addr_hi} }
            });
            sys.memory[addr_hi] = val;
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(sys.memory[addr_hi] == val + 1);
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 5);
            REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
        }

        SECTION("Zero Page, X") {
            Byte opcode = 0xF6;
            sys.load_program({
                { 0x8000, {opcode, addr_hi} }
            });
            sys.processor.registerSet(Mos6502::Register::X, addr_offset);
            sys.memory[addr_hi + addr_offset] = val;
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(sys.memory[addr_hi + addr_offset] == val + 1);
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 6);
            REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
        }

        SECTION("Absolute") {
            Byte opcode = 0xEE;
            sys.load_program({
                { 0x8000, {opcode, 0x00, addr_hi} }
            });
            sys.memory[addr_hi << 8] = val;
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(sys.memory[addr_hi << 8] == val + 1);
            REQUIRE(state.pc == 0x8003);
            REQUIRE(state.totalcycles == 6);
            REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
        }

        SECTION("Absolute, X") {
            Byte opcode = 0xFE;
            sys.load_program({
                { 0x8000, {opcode, 0x00, addr_hi} }
            });
            sys.processor.registerSet(Mos6502::Register::X, addr_offset);
            sys.memory[(addr_hi << 8) + addr_offset] = val;
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(sys.memory[(addr_hi << 8) + addr_offset] == val + 1);
            REQUIRE(state.pc == 0x8003);
            REQUIRE(state.totalcycles == 7);
            REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
        }

        SECTION("Absolute, X (page cross)") {
            Byte opcode = 0xFE;
            sys.load_program({
                { 0x8000, {opcode, 0xFF, addr_hi} }
            });
            sys.processor.registerSet(Mos6502::Register::X, addr_offset);
            sys.memory[((addr_hi + 1) << 8) + (addr_offset - 1)] = val;
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(sys.memory[((addr_hi + 1) << 8) + (addr_offset - 1)] == val + 1);
            REQUIRE(state.pc == 0x8003);
            REQUIRE(state.totalcycles == 8);
            REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
        }
    }

    SECTION("Flag Behavior") {
        Byte opcode = 0xE6;
        sys.load_program({
            { 0x8000, {opcode, addr_hi} }
        });

        SECTION("Negative flag is set when result >= 0x80") {
            sys.memory[addr_hi] = 0x90;
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(sys.memory[addr_hi] == 0x91);
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 5);
            REQUIRE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));

        }

        SECTION("Zero flag is set when result == 0") {
            sys.memory[addr_hi] = 0xFF;
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(sys.memory[addr_hi] == 0x00);
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 5);
            REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE(flag_status(state.status, FLAG_ZERO));

        }
    }
}

TEST_CASE("Instruction INX", "[Processor][INX]") {
    CpuTestFixture sys;
    Byte opcode = 0xE8;
    Byte val = 0x40;
    sys.load_program({
        { 0x8000, {opcode} }
    });

    SECTION("General Behavior") {
        sys.processor.registerSet(Mos6502::Register::X, val);
        sys.processor.step();
        auto state = sys.state();
        REQUIRE(state.x == val + 1);
        REQUIRE(state.pc == 0x8001);
        REQUIRE(state.totalcycles == 2);
        REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
        REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
    }

    SECTION("Flag Behavior") {
        SECTION("Negative flag is set when result >= 0x80") {
            sys.processor.registerSet(Mos6502::Register::X, 0x80);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.x == 0x81);
            REQUIRE(state.pc == 0x8001);
            REQUIRE(state.totalcycles == 2);
            REQUIRE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
        }

        SECTION("Zero flag is set when result == 0") {
            sys.processor.registerSet(Mos6502::Register::X, 0xFF);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.x == 0x00);
            REQUIRE(state.pc == 0x8001);
            REQUIRE(state.totalcycles == 2);
            REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE(flag_status(state.status, FLAG_ZERO));
        }
    }
}

TEST_CASE("Instruction INY", "[Processor][INY]") {
    CpuTestFixture sys;
    Byte opcode = 0xC8;
    Byte val = 0x40;
    sys.load_program({
        { 0x8000, {opcode} }
    });

    SECTION("General Behavior") {
        sys.processor.registerSet(Mos6502::Register::Y, val);
        sys.processor.step();
        auto state = sys.state();
        REQUIRE(state.y == val + 1);
        REQUIRE(state.pc == 0x8001);
        REQUIRE(state.totalcycles == 2);
        REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
        REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
    }

    SECTION("Flag Behavior") {
        SECTION("Negative flag is set when result >= 0x80") {
            sys.processor.registerSet(Mos6502::Register::Y, 0x80);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.y == 0x81);
            REQUIRE(state.pc == 0x8001);
            REQUIRE(state.totalcycles == 2);
            REQUIRE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
        }

        SECTION("Zero flag is set when result == 0") {
            sys.processor.registerSet(Mos6502::Register::Y, 0xFF);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.y == 0x00);
            REQUIRE(state.pc == 0x8001);
            REQUIRE(state.totalcycles == 2);
            REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE(flag_status(state.status, FLAG_ZERO));
        }
    }
}

/**********************************
 * Group: Arithmetic Instructions *
 * ADC, SBC                       *
 **********************************/

TEST_CASE("Instruction ADC", "[Processor][ADC]") {
    CpuTestFixture sys;

    SECTION("Addressing Modes") {
        const Byte addr_hi = 0x20;
        const Byte addr_offset = 0x05;
        const Byte reg_a = 0x20;
        const Byte operand = 0x20;
        const Byte result = 0x40;

        SECTION("Immediate") {
            Byte opcode = 0x69;
            sys.load_program({
                { 0x8000, {opcode, operand} }
            });
            sys.processor.registerSet(Mos6502::Register::A, reg_a);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.a == result);
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 2);
            REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
            REQUIRE_FALSE(flag_status(state.status, FLAG_CARRY));
            REQUIRE_FALSE(flag_status(state.status, FLAG_OVERFLOW));
        }

        SECTION("Zero Page") {
            Byte opcode = 0x65;
            sys.load_program({
                { 0x8000, {opcode, addr_hi} },
                { addr_hi, {operand} }
            });
            sys.processor.registerSet(Mos6502::Register::A, reg_a);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.a == result);
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 3);
            REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
            REQUIRE_FALSE(flag_status(state.status, FLAG_CARRY));
            REQUIRE_FALSE(flag_status(state.status, FLAG_OVERFLOW));
        }

        SECTION("Zero Page, X") {
            Byte opcode = 0x75;
            sys.load_program({
                { 0x8000, {opcode, addr_hi} },
                { addr_hi + addr_offset, {operand} }
            });
            sys.processor.registerSet(Mos6502::Register::A, reg_a);
            sys.processor.registerSet(Mos6502::Register::X, addr_offset);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.a == result);
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 4);
            REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
            REQUIRE_FALSE(flag_status(state.status, FLAG_CARRY));
            REQUIRE_FALSE(flag_status(state.status, FLAG_OVERFLOW));
        }

        SECTION("Absolute") {
            Byte opcode = 0x6D;
            sys.load_program({
                { 0x8000, {opcode, 0x00, addr_hi} },
                { addr_hi << 8, {operand} }
            });
            sys.processor.registerSet(Mos6502::Register::A, reg_a);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.a == result);
            REQUIRE(state.pc == 0x8003);
            REQUIRE(state.totalcycles == 4);
            REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
            REQUIRE_FALSE(flag_status(state.status, FLAG_CARRY));
            REQUIRE_FALSE(flag_status(state.status, FLAG_OVERFLOW));
        }

        SECTION("Absolute, X") {
            Byte opcode = 0x7D;

            SECTION("No Page Cross") {
                sys.load_program({
                    { 0x8000, {opcode, 0x00, addr_hi} },
                    { (addr_hi << 8) + addr_offset, {operand} }
                });
                sys.processor.registerSet(Mos6502::Register::A, reg_a);
                sys.processor.registerSet(Mos6502::Register::X, addr_offset);
                sys.processor.step();
                auto state = sys.state();
                REQUIRE(state.a == result);
                REQUIRE(state.pc == 0x8003);
                REQUIRE(state.totalcycles == 4);
                REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
                REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
                REQUIRE_FALSE(flag_status(state.status, FLAG_CARRY));
                REQUIRE_FALSE(flag_status(state.status, FLAG_OVERFLOW));
            }

            SECTION("Page Cross") {
                sys.load_program({
                    { 0x8000, {opcode, 0xFF, addr_hi} },
                    { (addr_hi << 8) + 0xFF + addr_offset, {operand} }
                });
                sys.processor.registerSet(Mos6502::Register::A, reg_a);
                sys.processor.registerSet(Mos6502::Register::X, addr_offset);
                sys.processor.step();
                auto state = sys.state();
                REQUIRE(state.a == result);
                REQUIRE(state.pc == 0x8003);
                REQUIRE(state.totalcycles == 5);
                REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
                REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
                REQUIRE_FALSE(flag_status(state.status, FLAG_CARRY));
                REQUIRE_FALSE(flag_status(state.status, FLAG_OVERFLOW));
            }
        }

        SECTION("Absolute, Y") {
            Byte opcode = 0x79;

            SECTION("No Page Cross") {
                sys.load_program({
                    { 0x8000, {opcode, 0x00, addr_hi} },
                    { (addr_hi << 8) + addr_offset, {operand} }
                });
                sys.processor.registerSet(Mos6502::Register::A, reg_a);
                sys.processor.registerSet(Mos6502::Register::Y, addr_offset);
                sys.processor.step();
                auto state = sys.state();
                REQUIRE(state.a == result);
                REQUIRE(state.pc == 0x8003);
                REQUIRE(state.totalcycles == 4);
                REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
                REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
                REQUIRE_FALSE(flag_status(state.status, FLAG_CARRY));
                REQUIRE_FALSE(flag_status(state.status, FLAG_OVERFLOW));
            }

            SECTION("Page Cross") {
                sys.load_program({
                    { 0x8000, {opcode, 0xFF, addr_hi} },
                    { (addr_hi << 8) + 0xFF + addr_offset, {operand} }
                });
                sys.processor.registerSet(Mos6502::Register::A, reg_a);
                sys.processor.registerSet(Mos6502::Register::Y, addr_offset);
                sys.processor.step();
                auto state = sys.state();
                REQUIRE(state.a == result);
                REQUIRE(state.pc == 0x8003);
                REQUIRE(state.totalcycles == 5);
                REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
                REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
                REQUIRE_FALSE(flag_status(state.status, FLAG_CARRY));
                REQUIRE_FALSE(flag_status(state.status, FLAG_OVERFLOW));
            }
        }

        SECTION("Indirect, X") {
            Byte opcode = 0x61;
            sys.load_program({
                { 0x8000, {opcode, addr_hi} },
                { addr_hi + addr_offset, {0x00, addr_hi} },
                { addr_hi << 8, {operand} }
            });
            sys.processor.registerSet(Mos6502::Register::A, reg_a);
            sys.processor.registerSet(Mos6502::Register::X, addr_offset);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.a == result);
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 6);
            REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
            REQUIRE_FALSE(flag_status(state.status, FLAG_CARRY));
            REQUIRE_FALSE(flag_status(state.status, FLAG_OVERFLOW));
        }

        SECTION("Indirect, Y") {
            Byte opcode = 0x71;

            SECTION("No Page Cross") {
                sys.load_program({
                    { 0x8000, {opcode, addr_hi} },
                    { addr_hi, {0x00, addr_hi} },
                    { (addr_hi << 8) + addr_offset, {operand} }
                });
                sys.processor.registerSet(Mos6502::Register::A, reg_a);
                sys.processor.registerSet(Mos6502::Register::Y, addr_offset);
                sys.processor.step();
                auto state = sys.state();
                REQUIRE(state.a == result);
                REQUIRE(state.pc == 0x8002);
                REQUIRE(state.totalcycles == 5);
                REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
                REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
                REQUIRE_FALSE(flag_status(state.status, FLAG_CARRY));
                REQUIRE_FALSE(flag_status(state.status, FLAG_OVERFLOW));
            }

            SECTION("Page Cross") {
                sys.load_program({
                    { 0x8000, {opcode, addr_hi} },
                    { addr_hi, {0xFF, addr_hi} },
                    { (addr_hi << 8) + 0xFF + addr_offset, {operand} }
                });
                sys.processor.registerSet(Mos6502::Register::A, reg_a);
                sys.processor.registerSet(Mos6502::Register::Y, addr_offset);
                sys.processor.step();
                auto state = sys.state();
                REQUIRE(state.a == result);
                REQUIRE(state.pc == 0x8002);
                REQUIRE(state.totalcycles == 6);
                REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
                REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
                REQUIRE_FALSE(flag_status(state.status, FLAG_CARRY));
                REQUIRE_FALSE(flag_status(state.status, FLAG_OVERFLOW));
            }
        }
    }

    SECTION("Binary Arithmetic & Flags") {
        Byte opcode = 0x69; // Immediate address

        SECTION("Addition Without Carry") {
            Byte reg_a = 0x20;
            Byte operand = 0x20;
            Byte result = 0x40;
            sys.load_program({
                { 0x8000, {opcode, operand} },
            });
            sys.processor.registerSet(Mos6502::Register::A, reg_a);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.a == result);
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 2);
            REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
            REQUIRE_FALSE(flag_status(state.status, FLAG_CARRY));
            REQUIRE_FALSE(flag_status(state.status, FLAG_OVERFLOW));
        }

        SECTION("Addition With Incoming Carry") {
            Byte reg_a = 0x20;
            Byte operand = 0x20;
            Byte result = 0x41;
            sys.load_program({
                { 0x8000, {opcode, operand} },
            });
            sys.processor.registerSet(Mos6502::Register::A, reg_a);
            sys.processor.registerSet(Mos6502::Register::Status, FLAG_CARRY);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.a == result);
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 2);
            REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
            REQUIRE_FALSE(flag_status(state.status, FLAG_CARRY));
            REQUIRE_FALSE(flag_status(state.status, FLAG_OVERFLOW));
        }

        SECTION("Result Zero Flag Set") {
            Byte reg_a = 0x00;
            Byte operand = 0x00;
            Byte result = 0x00;
            sys.load_program({
                { 0x8000, {opcode, operand} },
            });
            sys.processor.registerSet(Mos6502::Register::A, reg_a);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.a == result);
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 2);
            REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE(flag_status(state.status, FLAG_ZERO));
            REQUIRE_FALSE(flag_status(state.status, FLAG_CARRY));
            REQUIRE_FALSE(flag_status(state.status, FLAG_OVERFLOW));
        }

        SECTION("Result Carry Flag Set") {
            Byte reg_a = 0xFF;
            Byte operand = 0x02;
            Byte result = 0x01;
            sys.load_program({
                { 0x8000, {opcode, operand} },
            });
            sys.processor.registerSet(Mos6502::Register::A, reg_a);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.a == result);
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 2);
            REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
            REQUIRE(flag_status(state.status, FLAG_CARRY));
            REQUIRE_FALSE(flag_status(state.status, FLAG_OVERFLOW));
        }

        SECTION("Result Negative Flag Set") {
            Byte reg_a = 0x81;
            Byte operand = 0x02;
            Byte result = 0x83;
            sys.load_program({
                { 0x8000, {opcode, operand} },
            });
            sys.processor.registerSet(Mos6502::Register::A, reg_a);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.a == result);
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 2);
            REQUIRE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
            REQUIRE_FALSE(flag_status(state.status, FLAG_CARRY));
            REQUIRE_FALSE(flag_status(state.status, FLAG_OVERFLOW));
        }

        SECTION("Result Overflow Flag Set (From Positive Values)") {
            Byte reg_a = 0x7F;
            Byte operand = 0x02;
            Byte result = 0x81;
            sys.load_program({
                { 0x8000, {opcode, operand} },
            });
            sys.processor.registerSet(Mos6502::Register::A, reg_a);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.a == result);
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 2);
            REQUIRE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
            REQUIRE_FALSE(flag_status(state.status, FLAG_CARRY));
            REQUIRE(flag_status(state.status, FLAG_OVERFLOW));
        }

        SECTION("Result Overflow Flag Set (From Negative Values)") {
            Byte reg_a = 0x80;
            Byte operand = 0x81;
            Byte result = 0x01;
            sys.load_program({
                { 0x8000, {opcode, operand} },
            });
            sys.processor.registerSet(Mos6502::Register::A, reg_a);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.a == result);
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 2);
            REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
            REQUIRE(flag_status(state.status, FLAG_CARRY));
            REQUIRE(flag_status(state.status, FLAG_OVERFLOW));
        }
    }

    SECTION("Decimal (BCD) Mode Arithmetic") {
        Byte opcode = 0x69; // Immediate mode

        SECTION("Addition Without Incoming Carry") {
            Byte reg_a = 0x12;
            Byte operand = 0x34;
            Byte result = 0x46;
            sys.load_program({
                { 0x8000, {opcode, operand} },
            });
            sys.processor.registerSet(Mos6502::Register::A, reg_a);
            sys.processor.registerSet(Mos6502::Register::Status, FLAG_DECIMAL);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.a == result);
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 2);
            REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
            REQUIRE_FALSE(flag_status(state.status, FLAG_CARRY));
            REQUIRE_FALSE(flag_status(state.status, FLAG_OVERFLOW));
        }

        SECTION("Addition With Incoming Carry") {
            Byte reg_a = 0x12;
            Byte operand = 0x34;
            Byte result = 0x47;
            sys.load_program({
                { 0x8000, {opcode, operand} },
            });
            sys.processor.registerSet(Mos6502::Register::A, reg_a);
            sys.processor.registerSet(Mos6502::Register::Status, FLAG_DECIMAL | FLAG_CARRY);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.a == result);
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 2);
            REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
            REQUIRE_FALSE(flag_status(state.status, FLAG_CARRY));
            REQUIRE_FALSE(flag_status(state.status, FLAG_OVERFLOW));
        }

        SECTION("Low Nibble Correction") {
            Byte reg_a = 0x08;
            Byte operand = 0x05;
            Byte result = 0x13;
            sys.load_program({
                { 0x8000, {opcode, operand} },
            });
            sys.processor.registerSet(Mos6502::Register::A, reg_a);
            sys.processor.registerSet(Mos6502::Register::Status, FLAG_DECIMAL);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.a == result);
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 2);
            REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
            REQUIRE_FALSE(flag_status(state.status, FLAG_CARRY));
            REQUIRE_FALSE(flag_status(state.status, FLAG_OVERFLOW));
        }

        SECTION("High Nibble Correction") {
            Byte reg_a = 0x50;
            Byte operand = 0x51;
            Byte result = 0x01;
            sys.load_program({
                { 0x8000, {opcode, operand} },
            });
            sys.processor.registerSet(Mos6502::Register::A, reg_a);
            sys.processor.registerSet(Mos6502::Register::Status, FLAG_DECIMAL);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.a == result);
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 2);
            REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
            REQUIRE(flag_status(state.status, FLAG_CARRY));
            REQUIRE_FALSE(flag_status(state.status, FLAG_OVERFLOW));
        }
    }
}

TEST_CASE("Instruction SBC", "[Processor][SBC]") {
    CpuTestFixture sys;

    SECTION("Addressing Modes") {
        const Byte addr_hi = 0x20;
        const Byte addr_offset = 0x05;
        const Byte reg_a = 0x40;
        const Byte operand = 0x20;
        const Byte result = 0x20;

        SECTION("Immediate") {
            Byte opcode = 0xE9;
            sys.load_program({
                { 0x8000, {opcode, operand} }
            });
            sys.processor.registerSet(Mos6502::Register::A, reg_a);
            sys.processor.registerSet(Mos6502::Register::Status, FLAG_CARRY);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.a == result);
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 2);
            REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
            REQUIRE(flag_status(state.status, FLAG_CARRY));
            REQUIRE_FALSE(flag_status(state.status, FLAG_OVERFLOW));
        }

        SECTION("Zero Page") {
            Byte opcode = 0xE5;
            sys.load_program({
                { 0x8000, {opcode, addr_hi} },
                { addr_hi, {operand} }
            });
            sys.processor.registerSet(Mos6502::Register::A, reg_a);
            sys.processor.registerSet(Mos6502::Register::Status, FLAG_CARRY);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.a == result);
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 3);
            REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
            REQUIRE(flag_status(state.status, FLAG_CARRY));
            REQUIRE_FALSE(flag_status(state.status, FLAG_OVERFLOW));
        }

        SECTION("Zero Page, X") {
            Byte opcode = 0xF5;
            sys.load_program({
                { 0x8000, {opcode, addr_hi} },
                { addr_hi + addr_offset, {operand} }
            });
            sys.processor.registerSet(Mos6502::Register::A, reg_a);
            sys.processor.registerSet(Mos6502::Register::X, addr_offset);
            sys.processor.registerSet(Mos6502::Register::Status, FLAG_CARRY);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.a == result);
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 4);
            REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
            REQUIRE(flag_status(state.status, FLAG_CARRY));
            REQUIRE_FALSE(flag_status(state.status, FLAG_OVERFLOW));
        }

        SECTION("Absolute") {
            Byte opcode = 0xED;
            sys.load_program({
                { 0x8000, {opcode, 0x00, addr_hi} },
                { addr_hi << 8, {operand} }
            });
            sys.processor.registerSet(Mos6502::Register::A, reg_a);
            sys.processor.registerSet(Mos6502::Register::Status, FLAG_CARRY);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.a == result);
            REQUIRE(state.pc == 0x8003);
            REQUIRE(state.totalcycles == 4);
            REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
            REQUIRE(flag_status(state.status, FLAG_CARRY));
            REQUIRE_FALSE(flag_status(state.status, FLAG_OVERFLOW));
        }

        SECTION("Absolute, X") {
            Byte opcode = 0xFD;

            SECTION("No Page Cross") {
                sys.load_program({
                    { 0x8000, {opcode, 0x00, addr_hi} },
                    { (addr_hi << 8) + addr_offset, {operand} }
                });
                sys.processor.registerSet(Mos6502::Register::A, reg_a);
                sys.processor.registerSet(Mos6502::Register::X, addr_offset);
                sys.processor.registerSet(Mos6502::Register::Status, FLAG_CARRY);
                sys.processor.step();
                auto state = sys.state();
                REQUIRE(state.a == result);
                REQUIRE(state.pc == 0x8003);
                REQUIRE(state.totalcycles == 4);
                REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
                REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
                REQUIRE(flag_status(state.status, FLAG_CARRY));
                REQUIRE_FALSE(flag_status(state.status, FLAG_OVERFLOW));
            }

            SECTION("Page Cross") {
                sys.load_program({
                    { 0x8000, {opcode, 0xFF, addr_hi} },
                    { (addr_hi << 8) + 0xFF + addr_offset, {operand} }
                });
                sys.processor.registerSet(Mos6502::Register::A, reg_a);
                sys.processor.registerSet(Mos6502::Register::X, addr_offset);
                sys.processor.registerSet(Mos6502::Register::Status, FLAG_CARRY);
                sys.processor.step();
                auto state = sys.state();
                REQUIRE(state.a == result);
                REQUIRE(state.pc == 0x8003);
                REQUIRE(state.totalcycles == 5);
                REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
                REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
                REQUIRE(flag_status(state.status, FLAG_CARRY));
                REQUIRE_FALSE(flag_status(state.status, FLAG_OVERFLOW));
            }
        }

        SECTION("Absolute, Y") {
            Byte opcode = 0xF9;

            SECTION("No Page Cross") {
                sys.load_program({
                    { 0x8000, {opcode, 0x00, addr_hi} },
                    { (addr_hi << 8) + addr_offset, {operand} }
                });
                sys.processor.registerSet(Mos6502::Register::A, reg_a);
                sys.processor.registerSet(Mos6502::Register::Y, addr_offset);
                sys.processor.registerSet(Mos6502::Register::Status, FLAG_CARRY);
                sys.processor.step();
                auto state = sys.state();
                REQUIRE(state.a == result);
                REQUIRE(state.pc == 0x8003);
                REQUIRE(state.totalcycles == 4);
                REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
                REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
                REQUIRE(flag_status(state.status, FLAG_CARRY));
                REQUIRE_FALSE(flag_status(state.status, FLAG_OVERFLOW));
            }

            SECTION("Page Cross") {
                sys.load_program({
                    { 0x8000, {opcode, 0xFF, addr_hi} },
                    { (addr_hi << 8) + 0xFF + addr_offset, {operand} }
                });
                sys.processor.registerSet(Mos6502::Register::A, reg_a);
                sys.processor.registerSet(Mos6502::Register::Y, addr_offset);
                sys.processor.registerSet(Mos6502::Register::Status, FLAG_CARRY);
                sys.processor.step();
                auto state = sys.state();
                REQUIRE(state.a == result);
                REQUIRE(state.pc == 0x8003);
                REQUIRE(state.totalcycles == 5);
                REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
                REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
                REQUIRE(flag_status(state.status, FLAG_CARRY));
                REQUIRE_FALSE(flag_status(state.status, FLAG_OVERFLOW));
            }
        }

        SECTION("Indirect, X") {
            Byte opcode = 0xE1;
            sys.load_program({
                { 0x8000, {opcode, addr_hi} },
                { addr_hi + addr_offset, {0x00, addr_hi} },
                { addr_hi << 8, {operand} }
            });
            sys.processor.registerSet(Mos6502::Register::A, reg_a);
            sys.processor.registerSet(Mos6502::Register::X, addr_offset);
            sys.processor.registerSet(Mos6502::Register::Status, FLAG_CARRY);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.a == result);
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 6);
            REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
            REQUIRE(flag_status(state.status, FLAG_CARRY));
            REQUIRE_FALSE(flag_status(state.status, FLAG_OVERFLOW));
        }

        SECTION("Indirect, Y") {
            Byte opcode = 0xF1;

            SECTION("No Page Cross") {
                sys.load_program({
                    { 0x8000, {opcode, addr_hi} },
                    { addr_hi, {0x00, addr_hi} },
                    { (addr_hi << 8) + addr_offset, {operand} }
                });
                sys.processor.registerSet(Mos6502::Register::A, reg_a);
                sys.processor.registerSet(Mos6502::Register::Y, addr_offset);
                sys.processor.registerSet(Mos6502::Register::Status, FLAG_CARRY);
                sys.processor.step();
                auto state = sys.state();
                REQUIRE(state.a == result);
                REQUIRE(state.pc == 0x8002);
                REQUIRE(state.totalcycles == 5);
                REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
                REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
                REQUIRE(flag_status(state.status, FLAG_CARRY));
                REQUIRE_FALSE(flag_status(state.status, FLAG_OVERFLOW));
            }

            SECTION("Page Cross") {
                sys.load_program({
                    { 0x8000, {opcode, addr_hi} },
                    { addr_hi, {0xFF, addr_hi} },
                    { (addr_hi << 8) + 0xFF + addr_offset, {operand} }
                });
                sys.processor.registerSet(Mos6502::Register::A, reg_a);
                sys.processor.registerSet(Mos6502::Register::Y, addr_offset);
                sys.processor.registerSet(Mos6502::Register::Status, FLAG_CARRY);
                sys.processor.step();
                auto state = sys.state();
                REQUIRE(state.a == result);
                REQUIRE(state.pc == 0x8002);
                REQUIRE(state.totalcycles == 6);
                REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
                REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
                REQUIRE(flag_status(state.status, FLAG_CARRY));
                REQUIRE_FALSE(flag_status(state.status, FLAG_OVERFLOW));
            }
        }
    }

    SECTION("Binary Arithmetic & Flags") {
        Byte opcode = 0xE9; // Immediate address

        SECTION("Subtraction Without Carry") {
            Byte reg_a = 0x40;
            Byte operand = 0x20;
            Byte result = 0x1F;
            sys.load_program({
                { 0x8000, {opcode, operand} },
            });
            sys.processor.registerSet(Mos6502::Register::A, reg_a);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.a == result);
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 2);
            REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
            REQUIRE(flag_status(state.status, FLAG_CARRY));
            REQUIRE_FALSE(flag_status(state.status, FLAG_OVERFLOW));
        }

        SECTION("Subtraction With Incoming Carry") {
            Byte reg_a = 0x40;
            Byte operand = 0x20;
            Byte result = 0x20;
            sys.load_program({
                { 0x8000, {opcode, operand} },
            });
            sys.processor.registerSet(Mos6502::Register::A, reg_a);
            sys.processor.registerSet(Mos6502::Register::Status, FLAG_CARRY);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.a == result);
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 2);
            REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
            REQUIRE(flag_status(state.status, FLAG_CARRY));
            REQUIRE_FALSE(flag_status(state.status, FLAG_OVERFLOW));
        }

        SECTION("Result Zero Flag Set") {
            Byte reg_a = 0x00;
            Byte operand = 0x00;
            Byte result = 0x00;
            sys.load_program({
                { 0x8000, {opcode, operand} },
            });
            sys.processor.registerSet(Mos6502::Register::A, reg_a);
            sys.processor.registerSet(Mos6502::Register::Status, FLAG_CARRY);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.a == result);
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 2);
            REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE(flag_status(state.status, FLAG_ZERO));
            REQUIRE(flag_status(state.status, FLAG_CARRY));
            REQUIRE_FALSE(flag_status(state.status, FLAG_OVERFLOW));
        }

        SECTION("Result Carry Flag Not Set") {
            Byte reg_a = 0x00;
            Byte operand = 0x02;
            Byte result = 0xFE;
            sys.load_program({
                { 0x8000, {opcode, operand} },
            });
            sys.processor.registerSet(Mos6502::Register::A, reg_a);
            sys.processor.registerSet(Mos6502::Register::Status, FLAG_CARRY);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.a == result);
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 2);
            REQUIRE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
            REQUIRE_FALSE(flag_status(state.status, FLAG_CARRY));
            REQUIRE(flag_status(state.status, FLAG_OVERFLOW));
        }

        SECTION("Result Negative Flag Set") {
            Byte reg_a = 0x85;
            Byte operand = 0x02;
            Byte result = 0x83;
            sys.load_program({
                { 0x8000, {opcode, operand} },
            });
            sys.processor.registerSet(Mos6502::Register::A, reg_a);
            sys.processor.registerSet(Mos6502::Register::Status, FLAG_CARRY);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.a == result);
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 2);
            REQUIRE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
            REQUIRE(flag_status(state.status, FLAG_CARRY));
            REQUIRE_FALSE(flag_status(state.status, FLAG_OVERFLOW));
        }

        SECTION("Result Overflow Flag Set (From Positive Values)") {
            Byte reg_a = 0x05;
            Byte operand = 0x08;
            Byte result = 0xFD;
            sys.load_program({
                { 0x8000, {opcode, operand} },
            });
            sys.processor.registerSet(Mos6502::Register::A, reg_a);
            sys.processor.registerSet(Mos6502::Register::Status, FLAG_CARRY);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.a == result);
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 2);
            REQUIRE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
            REQUIRE_FALSE(flag_status(state.status, FLAG_CARRY));
            REQUIRE(flag_status(state.status, FLAG_OVERFLOW));
        }

        SECTION("Result Overflow Flag Set (From Negative Values)") {
            Byte reg_a = 0x81;
            Byte operand = 0x80;
            Byte result = 0x01;
            sys.load_program({
                { 0x8000, {opcode, operand} },
            });
            sys.processor.registerSet(Mos6502::Register::A, reg_a);
            sys.processor.registerSet(Mos6502::Register::Status, FLAG_CARRY);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.a == result);
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 2);
            REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
            REQUIRE(flag_status(state.status, FLAG_CARRY));
            REQUIRE(flag_status(state.status, FLAG_OVERFLOW));
        }
    }

    SECTION("Decimal (BCD) Mode Arithmetic") {
        Byte opcode = 0xE9; // Immediate mode

        SECTION("Subtraction Without Incoming Carry") {
            Byte reg_a = 0x34;
            Byte operand = 0x12;
            Byte result = 0x21;
            sys.load_program({
                { 0x8000, {opcode, operand} },
            });
            sys.processor.registerSet(Mos6502::Register::A, reg_a);
            sys.processor.registerSet(Mos6502::Register::Status, FLAG_DECIMAL);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.a == result);
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 2);
            REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
            REQUIRE(flag_status(state.status, FLAG_CARRY));
            REQUIRE_FALSE(flag_status(state.status, FLAG_OVERFLOW));
        }

        SECTION("Subtraction With Incoming Carry") {
            Byte reg_a = 0x34;
            Byte operand = 0x12;
            Byte result = 0x22;
            sys.load_program({
                { 0x8000, {opcode, operand} },
            });
            sys.processor.registerSet(Mos6502::Register::A, reg_a);
            sys.processor.registerSet(Mos6502::Register::Status, FLAG_DECIMAL | FLAG_CARRY);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.a == result);
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 2);
            REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
            REQUIRE(flag_status(state.status, FLAG_CARRY));
            REQUIRE_FALSE(flag_status(state.status, FLAG_OVERFLOW));
        }

        SECTION("Low Nibble Correction") {
            Byte reg_a = 0x11;
            Byte operand = 0x02;
            Byte result = 0x09;
            sys.load_program({
                { 0x8000, {opcode, operand} },
            });
            sys.processor.registerSet(Mos6502::Register::A, reg_a);
            sys.processor.registerSet(Mos6502::Register::Status, FLAG_DECIMAL | FLAG_CARRY);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.a == result);
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 2);
            REQUIRE_FALSE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
            REQUIRE(flag_status(state.status, FLAG_CARRY));
            REQUIRE_FALSE(flag_status(state.status, FLAG_OVERFLOW));
        }

        SECTION("High Nibble Correction") {
            Byte reg_a = 0x10;
            Byte operand = 0x20;
            Byte result = 0x90;
            sys.load_program({
                { 0x8000, {opcode, operand} },
            });
            sys.processor.registerSet(Mos6502::Register::A, reg_a);
            sys.processor.registerSet(Mos6502::Register::Status, FLAG_DECIMAL | FLAG_CARRY);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.a == result);
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 2);
            REQUIRE(flag_status(state.status, FLAG_NEGATIVE));
            REQUIRE_FALSE(flag_status(state.status, FLAG_ZERO));
            REQUIRE_FALSE(flag_status(state.status, FLAG_CARRY));
            REQUIRE(flag_status(state.status, FLAG_OVERFLOW));

        }
    }
}

/*****************************************
 * Group: Logical Operation Instructions *
 * AND, EOR, ORA                         *
 *****************************************/

TEST_CASE("Instruction AND", "[Processor][AND]") {
    SECTION("Addressing Modes") {
        CpuTestFixture sys;
        const Byte reg_a = 0b01100110;
        const Byte operand = 0b11001100;
        const Byte result = 0b01000100;
        const Byte addr_hi = 0x20;
        const Byte addr_offset = 0x05;

        SECTION("Immediate") {
            Byte opcode = 0x29;
            sys.load_program({
                { 0x8000, {opcode, operand} }
            });
            sys.processor.registerSet(Mos6502::Register::A, reg_a);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 2);
            REQUIRE(state.a == result);
            REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
            REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
        }

        SECTION("Zero Page") {
            Byte opcode = 0x25;
            sys.load_program({
                { 0x8000, {opcode, addr_hi} },
                { addr_hi, {operand} }
            });
            sys.processor.registerSet(Mos6502::Register::A, reg_a);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 3);
            REQUIRE(state.a == result);
            REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
            REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
        }

        SECTION("Zero Page, X") {
            Byte opcode = 0x35;
            sys.load_program({
                { 0x8000, {opcode, addr_hi} },
                { addr_hi + addr_offset, {operand} }
            });
            sys.processor.registerSet(Mos6502::Register::A, reg_a);
            sys.processor.registerSet(Mos6502::Register::X, addr_offset);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 4);
            REQUIRE(state.a == result);
            REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
            REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
        }

        SECTION("Absolute") {
            Byte opcode = 0x2D;
            sys.load_program({
                { 0x8000, {opcode, 0x00, addr_hi} },
                { addr_hi << 8, {operand} }
            });
            sys.processor.registerSet(Mos6502::Register::A, reg_a);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.pc == 0x8003);
            REQUIRE(state.totalcycles == 4);
            REQUIRE(state.a == result);
            REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
            REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
        }

        SECTION("Absolute, X") {
            Byte opcode = 0x3D;

            SECTION("No Page Cross") {
                sys.load_program({
                    { 0x8000, {opcode, 0x00, addr_hi} },
                    { (addr_hi << 8) + addr_offset, {operand} }
                });
                sys.processor.registerSet(Mos6502::Register::A, reg_a);
                sys.processor.registerSet(Mos6502::Register::X, addr_offset);
                sys.processor.step();
                auto state = sys.state();
                REQUIRE(state.pc == 0x8003);
                REQUIRE(state.totalcycles == 4);
                REQUIRE(state.a == result);
                REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
                REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
            }

            SECTION("Page Cross") {
                sys.load_program({
                    { 0x8000, {opcode, 0xFF, addr_hi} },
                    { (addr_hi << 8) + 0xFF + addr_offset, {operand} }
                });
                sys.processor.registerSet(Mos6502::Register::A, reg_a);
                sys.processor.registerSet(Mos6502::Register::X, addr_offset);
                sys.processor.step();
                auto state = sys.state();
                REQUIRE(state.pc == 0x8003);
                REQUIRE(state.totalcycles == 5);
                REQUIRE(state.a == result);
                REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
                REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
            }
        }

        SECTION("Absolute, Y") {
            Byte opcode = 0x39;

            SECTION("No Page Cross") {
                sys.load_program({
                    { 0x8000, {opcode, 0x00, addr_hi} },
                    { (addr_hi << 8) + addr_offset, {operand} }
                });
                sys.processor.registerSet(Mos6502::Register::A, reg_a);
                sys.processor.registerSet(Mos6502::Register::Y, addr_offset);
                sys.processor.step();
                auto state = sys.state();
                REQUIRE(state.pc == 0x8003);
                REQUIRE(state.totalcycles == 4);
                REQUIRE(state.a == result);
                REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
                REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
            }

            SECTION("Page Cross") {
                sys.load_program({
                    { 0x8000, {opcode, 0xFF, addr_hi} },
                    { (addr_hi << 8) + 0xFF + addr_offset, {operand} }
                });
                sys.processor.registerSet(Mos6502::Register::A, reg_a);
                sys.processor.registerSet(Mos6502::Register::Y, addr_offset);
                sys.processor.step();
                auto state = sys.state();
                REQUIRE(state.pc == 0x8003);
                REQUIRE(state.totalcycles == 5);
                REQUIRE(state.a == result);
                REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
                REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
            }
        }

        SECTION("Indirect, X") {
            Byte opcode = 0x21;
            sys.load_program({
                { 0x8000, {opcode, addr_hi} },
                { addr_hi + addr_offset, {0x00, addr_hi} },
                { addr_hi << 8, {operand} }
            });
            sys.processor.registerSet(Mos6502::Register::A, reg_a);
            sys.processor.registerSet(Mos6502::Register::X, addr_offset);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 6);
            REQUIRE(state.a == result);
            REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
            REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
        }

        SECTION("Indirect, Y") {
            Byte opcode = 0x31;

            SECTION("No Page Cross") {
                sys.load_program({
                    { 0x8000, {opcode, addr_hi} },
                    { addr_hi, {0x00, addr_hi} },
                    { (addr_hi << 8) + addr_offset, {operand} }
                });
                sys.processor.registerSet(Mos6502::Register::A, reg_a);
                sys.processor.registerSet(Mos6502::Register::Y, addr_offset);
                sys.processor.step();
                auto state = sys.state();
                REQUIRE(state.pc == 0x8002);
                REQUIRE(state.totalcycles == 5);
                REQUIRE(state.a == result);
                REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
                REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
            }

            SECTION("Page Cross") {
                sys.load_program({
                    { 0x8000, {opcode, addr_hi} },
                    { addr_hi, {0xFF, addr_hi} },
                    { (addr_hi << 8) + 0xFF + addr_offset, {operand} }
                });
                sys.processor.registerSet(Mos6502::Register::A, reg_a);
                sys.processor.registerSet(Mos6502::Register::Y, addr_offset);
                sys.processor.step();
                auto state = sys.state();
                REQUIRE(state.pc == 0x8002);
                REQUIRE(state.totalcycles == 6);
                REQUIRE(state.a == result);
                REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
                REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
            }
        }
    }

    SECTION("Flags") {
        CpuTestFixture sys;
        Byte opcode = 0x29;

        SECTION("Result Negative Flag Set") {
            sys.load_program({
                { 0x8000, {opcode, 0b11111111} }
            });
            sys.processor.registerSet(Mos6502::Register::A, 0b10000001);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 2);
            REQUIRE(state.a == 0b10000001);
            REQUIRE(flag_status(FLAG_NEGATIVE, state.status));
            REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
        }

        SECTION("Result Negative Flag Clear") {
            sys.load_program({
                { 0x8000, {opcode, 0b01111111} }
            });
            sys.processor.registerSet(Mos6502::Register::A, 0b10000001);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 2);
            REQUIRE(state.a == 0b00000001);
            REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
            REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
        }

        SECTION("Result Zero Flag Set") {
            sys.load_program({
                { 0x8000, {opcode, 0b10101010} }
            });
            sys.processor.registerSet(Mos6502::Register::A, 0b01010101);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 2);
            REQUIRE(state.a == 0b00000000);
            REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
            REQUIRE(flag_status(FLAG_ZERO, state.status));
        }

        SECTION("Result Zero Flag Clear") {
            sys.load_program({
                { 0x8000, {opcode, 0b10101011} }
            });
            sys.processor.registerSet(Mos6502::Register::A, 0b01010101);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 2);
            REQUIRE(state.a == 0b00000001);
            REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
            REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
        }
    }
}

TEST_CASE("Instruction EOR", "[Processor][EOR]") {
    SECTION("Addressing Modes") {
        CpuTestFixture sys;
        const Byte reg_a = 0b01100100;
        const Byte operand = 0b00101101;
        const Byte result = 0b01001001;
        const Byte addr_hi = 0x20;
        const Byte addr_offset = 0x05;

        SECTION("Immediate") {
            Byte opcode = 0x49;
            sys.load_program({
                { 0x8000, {opcode, operand} }
            });
            sys.processor.registerSet(Mos6502::Register::A, reg_a);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 2);
            REQUIRE(state.a == result);
            REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
            REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
        }

        SECTION("Zero Page") {
            Byte opcode = 0x45;
            sys.load_program({
                { 0x8000, {opcode, addr_hi} },
                { addr_hi, {operand} }
            });
            sys.processor.registerSet(Mos6502::Register::A, reg_a);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 3);
            REQUIRE(state.a == result);
            REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
            REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
        }

        SECTION("Zero Page, X") {
            Byte opcode = 0x55;
            sys.load_program({
                { 0x8000, {opcode, addr_hi} },
                { addr_hi + addr_offset, {operand} }
            });
            sys.processor.registerSet(Mos6502::Register::A, reg_a);
            sys.processor.registerSet(Mos6502::Register::X, addr_offset);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 4);
            REQUIRE(state.a == result);
            REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
            REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
        }

        SECTION("Absolute") {
            Byte opcode = 0x4D;
            sys.load_program({
                { 0x8000, {opcode, 0x00, addr_hi} },
                { addr_hi << 8, {operand} }
            });
            sys.processor.registerSet(Mos6502::Register::A, reg_a);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.pc == 0x8003);
            REQUIRE(state.totalcycles == 4);
            REQUIRE(state.a == result);
            REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
            REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
        }

        SECTION("Absolute, X") {
            Byte opcode = 0x5D;

            SECTION("No Page Cross") {
                sys.load_program({
                    { 0x8000, {opcode, 0x00, addr_hi} },
                    { (addr_hi << 8) + addr_offset, {operand} }
                });
                sys.processor.registerSet(Mos6502::Register::A, reg_a);
                sys.processor.registerSet(Mos6502::Register::X, addr_offset);
                sys.processor.step();
                auto state = sys.state();
                REQUIRE(state.pc == 0x8003);
                REQUIRE(state.totalcycles == 4);
                REQUIRE(state.a == result);
                REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
                REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
            }

            SECTION("Page Cross") {
                sys.load_program({
                    { 0x8000, {opcode, 0xFF, addr_hi} },
                    { (addr_hi << 8) + 0xFF + addr_offset, {operand} }
                });
                sys.processor.registerSet(Mos6502::Register::A, reg_a);
                sys.processor.registerSet(Mos6502::Register::X, addr_offset);
                sys.processor.step();
                auto state = sys.state();
                REQUIRE(state.pc == 0x8003);
                REQUIRE(state.totalcycles == 5);
                REQUIRE(state.a == result);
                REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
                REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
            }
        }

        SECTION("Absolute, Y") {
            Byte opcode = 0x59;

            SECTION("No Page Cross") {
                sys.load_program({
                    { 0x8000, {opcode, 0x00, addr_hi} },
                    { (addr_hi << 8) + addr_offset, {operand} }
                });
                sys.processor.registerSet(Mos6502::Register::A, reg_a);
                sys.processor.registerSet(Mos6502::Register::Y, addr_offset);
                sys.processor.step();
                auto state = sys.state();
                REQUIRE(state.pc == 0x8003);
                REQUIRE(state.totalcycles == 4);
                REQUIRE(state.a == result);
                REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
                REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
            }

            SECTION("Page Cross") {
                sys.load_program({
                    { 0x8000, {opcode, 0xFF, addr_hi} },
                    { (addr_hi << 8) + 0xFF + addr_offset, {operand} }
                });
                sys.processor.registerSet(Mos6502::Register::A, reg_a);
                sys.processor.registerSet(Mos6502::Register::Y, addr_offset);
                sys.processor.step();
                auto state = sys.state();
                REQUIRE(state.pc == 0x8003);
                REQUIRE(state.totalcycles == 5);
                REQUIRE(state.a == result);
                REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
                REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
            }
        }

        SECTION("Indirect, X") {
            Byte opcode = 0x41;
            sys.load_program({
                { 0x8000, {opcode, addr_hi} },
                { addr_hi + addr_offset, {0x00, addr_hi} },
                { addr_hi << 8, {operand} }
            });
            sys.processor.registerSet(Mos6502::Register::A, reg_a);
            sys.processor.registerSet(Mos6502::Register::X, addr_offset);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 6);
            REQUIRE(state.a == result);
            REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
            REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
        }

        SECTION("Indirect, Y") {
            Byte opcode = 0x51;

            SECTION("No Page Cross") {
                sys.load_program({
                    { 0x8000, {opcode, addr_hi} },
                    { addr_hi, {0x00, addr_hi} },
                    { (addr_hi << 8) + addr_offset, {operand} }
                });
                sys.processor.registerSet(Mos6502::Register::A, reg_a);
                sys.processor.registerSet(Mos6502::Register::Y, addr_offset);
                sys.processor.step();
                auto state = sys.state();
                REQUIRE(state.pc == 0x8002);
                REQUIRE(state.totalcycles == 5);
                REQUIRE(state.a == result);
                REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
                REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
            }

            SECTION("Page Cross") {
                sys.load_program({
                    { 0x8000, {opcode, addr_hi} },
                    { addr_hi, {0xFF, addr_hi} },
                    { (addr_hi << 8) + 0xFF + addr_offset, {operand} }
                });
                sys.processor.registerSet(Mos6502::Register::A, reg_a);
                sys.processor.registerSet(Mos6502::Register::Y, addr_offset);
                sys.processor.step();
                auto state = sys.state();
                REQUIRE(state.pc == 0x8002);
                REQUIRE(state.totalcycles == 6);
                REQUIRE(state.a == result);
                REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
                REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
            }
        }
    }

    SECTION("Flags") {
        CpuTestFixture sys;
        Byte opcode = 0x49;

        SECTION("Result Negative Flag Set") {
            sys.load_program({
                { 0x8000, {opcode, 0b11111111} }
            });
            sys.processor.registerSet(Mos6502::Register::A, 0b00000001);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 2);
            REQUIRE(state.a == 0b11111110);
            REQUIRE(flag_status(FLAG_NEGATIVE, state.status));
            REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
        }

        SECTION("Result Negative Flag Clear") {
            sys.load_program({
                { 0x8000, {opcode, 0b10000000} }
            });
            sys.processor.registerSet(Mos6502::Register::A, 0b10000001);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 2);
            REQUIRE(state.a == 0b00000001);
            REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
            REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
        }

        SECTION("Result Zero Flag Set") {
            sys.load_program({
                { 0x8000, {opcode, 0b11111111} }
            });
            sys.processor.registerSet(Mos6502::Register::A, 0b11111111);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 2);
            REQUIRE(state.a == 0b00000000);
            REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
            REQUIRE(flag_status(FLAG_ZERO, state.status));
        }

        SECTION("Result Zero Flag Clear") {
            sys.load_program({
                { 0x8000, {opcode, 0b01100110} }
            });
            sys.processor.registerSet(Mos6502::Register::A, 0b00100010);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 2);
            REQUIRE(state.a == 0b01000100);
            REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
            REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
        }
    }
}

TEST_CASE("Instruction ORA", "[Processor][ORA]") {
    SECTION("Addressing Modes") {
        CpuTestFixture sys;
        const Byte reg_a = 0b01100100;
        const Byte operand = 0b00101101;
        const Byte result = 0b01101101;
        const Byte addr_hi = 0x20;
        const Byte addr_offset = 0x05;

        SECTION("Immediate") {
            Byte opcode = 0x09;
            sys.load_program({
                { 0x8000, {opcode, operand} }
            });
            sys.processor.registerSet(Mos6502::Register::A, reg_a);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 2);
            REQUIRE(state.a == result);
            REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
            REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
        }

        SECTION("Zero Page") {
            Byte opcode = 0x05;
            sys.load_program({
                { 0x8000, {opcode, addr_hi} },
                { addr_hi, {operand} }
            });
            sys.processor.registerSet(Mos6502::Register::A, reg_a);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 3);
            REQUIRE(state.a == result);
            REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
            REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
        }

        SECTION("Zero Page, X") {
            Byte opcode = 0x15;
            sys.load_program({
                { 0x8000, {opcode, addr_hi} },
                { addr_hi + addr_offset, {operand} }
            });
            sys.processor.registerSet(Mos6502::Register::A, reg_a);
            sys.processor.registerSet(Mos6502::Register::X, addr_offset);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 4);
            REQUIRE(state.a == result);
            REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
            REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
        }

        SECTION("Absolute") {
            Byte opcode = 0x0D;
            sys.load_program({
                { 0x8000, {opcode, 0x00, addr_hi} },
                { addr_hi << 8, {operand} }
            });
            sys.processor.registerSet(Mos6502::Register::A, reg_a);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.pc == 0x8003);
            REQUIRE(state.totalcycles == 4);
            REQUIRE(state.a == result);
            REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
            REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
        }

        SECTION("Absolute, X") {
            Byte opcode = 0x1D;

            SECTION("No Page Cross") {
                sys.load_program({
                    { 0x8000, {opcode, 0x00, addr_hi} },
                    { (addr_hi << 8) + addr_offset, {operand} }
                });
                sys.processor.registerSet(Mos6502::Register::A, reg_a);
                sys.processor.registerSet(Mos6502::Register::X, addr_offset);
                sys.processor.step();
                auto state = sys.state();
                REQUIRE(state.pc == 0x8003);
                REQUIRE(state.totalcycles == 4);
                REQUIRE(state.a == result);
                REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
                REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
            }

            SECTION("Page Cross") {
                sys.load_program({
                    { 0x8000, {opcode, 0xFF, addr_hi} },
                    { (addr_hi << 8) + 0xFF + addr_offset, {operand} }
                });
                sys.processor.registerSet(Mos6502::Register::A, reg_a);
                sys.processor.registerSet(Mos6502::Register::X, addr_offset);
                sys.processor.step();
                auto state = sys.state();
                REQUIRE(state.pc == 0x8003);
                REQUIRE(state.totalcycles == 5);
                REQUIRE(state.a == result);
                REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
                REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
            }
        }

        SECTION("Absolute, Y") {
            Byte opcode = 0x19;

            SECTION("No Page Cross") {
                sys.load_program({
                    { 0x8000, {opcode, 0x00, addr_hi} },
                    { (addr_hi << 8) + addr_offset, {operand} }
                });
                sys.processor.registerSet(Mos6502::Register::A, reg_a);
                sys.processor.registerSet(Mos6502::Register::Y, addr_offset);
                sys.processor.step();
                auto state = sys.state();
                REQUIRE(state.pc == 0x8003);
                REQUIRE(state.totalcycles == 4);
                REQUIRE(state.a == result);
                REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
                REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
            }

            SECTION("Page Cross") {
                sys.load_program({
                    { 0x8000, {opcode, 0xFF, addr_hi} },
                    { (addr_hi << 8) + 0xFF + addr_offset, {operand} }
                });
                sys.processor.registerSet(Mos6502::Register::A, reg_a);
                sys.processor.registerSet(Mos6502::Register::Y, addr_offset);
                sys.processor.step();
                auto state = sys.state();
                REQUIRE(state.pc == 0x8003);
                REQUIRE(state.totalcycles == 5);
                REQUIRE(state.a == result);
                REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
                REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
            }
        }

        SECTION("Indirect, X") {
            Byte opcode = 0x01;
            sys.load_program({
                { 0x8000, {opcode, addr_hi} },
                { addr_hi + addr_offset, {0x00, addr_hi} },
                { addr_hi << 8, {operand} }
            });
            sys.processor.registerSet(Mos6502::Register::A, reg_a);
            sys.processor.registerSet(Mos6502::Register::X, addr_offset);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 6);
            REQUIRE(state.a == result);
            REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
            REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
        }

        SECTION("Indirect, Y") {
            Byte opcode = 0x11;

            SECTION("No Page Cross") {
                sys.load_program({
                    { 0x8000, {opcode, addr_hi} },
                    { addr_hi, {0x00, addr_hi} },
                    { (addr_hi << 8) + addr_offset, {operand} }
                });
                sys.processor.registerSet(Mos6502::Register::A, reg_a);
                sys.processor.registerSet(Mos6502::Register::Y, addr_offset);
                sys.processor.step();
                auto state = sys.state();
                REQUIRE(state.pc == 0x8002);
                REQUIRE(state.totalcycles == 5);
                REQUIRE(state.a == result);
                REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
                REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
            }

            SECTION("Page Cross") {
                sys.load_program({
                    { 0x8000, {opcode, addr_hi} },
                    { addr_hi, {0xFF, addr_hi} },
                    { (addr_hi << 8) + 0xFF + addr_offset, {operand} }
                });
                sys.processor.registerSet(Mos6502::Register::A, reg_a);
                sys.processor.registerSet(Mos6502::Register::Y, addr_offset);
                sys.processor.step();
                auto state = sys.state();
                REQUIRE(state.pc == 0x8002);
                REQUIRE(state.totalcycles == 6);
                REQUIRE(state.a == result);
                REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
                REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
            }
        }
    }

    SECTION("Flags") {
        CpuTestFixture sys;
        Byte opcode = 0x09;

        SECTION("Result Negative Flag Set") {
            sys.load_program({
                { 0x8000, {opcode, 0b11111111} }
            });
            sys.processor.registerSet(Mos6502::Register::A, 0b00000001);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 2);
            REQUIRE(state.a == 0b11111111);
            REQUIRE(flag_status(FLAG_NEGATIVE, state.status));
            REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
        }

        SECTION("Result Negative Flag Clear") {
            sys.load_program({
                { 0x8000, {opcode, 0b00000000} }
            });
            sys.processor.registerSet(Mos6502::Register::A, 0b00000001);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 2);
            REQUIRE(state.a == 0b00000001);
            REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
            REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
        }

        SECTION("Result Zero Flag Set") {
            sys.load_program({
                { 0x8000, {opcode, 0b00000000} }
            });
            sys.processor.registerSet(Mos6502::Register::A, 0b00000000);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 2);
            REQUIRE(state.a == 0b00000000);
            REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
            REQUIRE(flag_status(FLAG_ZERO, state.status));
        }

        SECTION("Result Zero Flag Clear") {
            sys.load_program({
                { 0x8000, {opcode, 0b01100110} }
            });
            sys.processor.registerSet(Mos6502::Register::A, 0b00100010);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 2);
            REQUIRE(state.a == 0b01100110);
            REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
            REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
        }
    }
}

/****************************************
 * Group: Shift and Rotate Instructions *
 ****************************************/

TEST_CASE("Instruction ASL", "[Processor][ASL]") {
    CpuTestFixture sys;
    const Byte addr_hi = 0x20;
    const Byte addr_offset = 0x05;
    const Byte value = 0b00100000;
    const Byte result = 0b01000000;

    SECTION("Addressing Modes") {
        SECTION("Accumulator") {
            const Byte opcode = 0x0A;
            sys.load_program({
                { 0x8000, {opcode} }
            });
            sys.processor.registerSet(Mos6502::Register::A, value);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.pc == 0x8001);
            REQUIRE(state.totalcycles == 2);
            REQUIRE(state.a == result);
            REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
            REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
            REQUIRE_FALSE(flag_status(FLAG_CARRY, state.status));
        }

        SECTION("Zero Page") {
            const Byte opcode = 0x06;
            const Word addr_effective = addr_hi;
            sys.load_program({
                { 0x8000, {opcode, addr_hi} },
                { addr_effective, {value} }
            });
            sys.memory[addr_effective] = value;
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 5);
            REQUIRE(sys.memory[addr_effective] == result);
            REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
            REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
            REQUIRE_FALSE(flag_status(FLAG_CARRY, state.status));
        }

        SECTION("Zero Page, X") {
            const Byte opcode = 0x16;
            const Word addr_effective = addr_hi + addr_offset;
            sys.load_program({
                { 0x8000, {opcode, addr_hi} },
                { addr_effective, {value} }
            });
            sys.processor.registerSet(Mos6502::Register::X, addr_offset);
            sys.memory[addr_effective] = value;
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 6);
            REQUIRE(sys.memory[addr_effective] == result);
            REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
            REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
            REQUIRE_FALSE(flag_status(FLAG_CARRY, state.status));
        }

        SECTION("Absolute") {
            const Byte opcode = 0x0E;
            const Word addr_effective = addr_hi << 8;
            sys.load_program({
                { 0x8000, {opcode, addr_effective & 0xFF, addr_effective >> 8} },
                { addr_effective, {value} }
            });
            sys.memory[addr_effective] = value;
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.pc == 0x8003);
            REQUIRE(state.totalcycles == 6);
            REQUIRE(sys.memory[addr_effective] == result);
            REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
            REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
            REQUIRE_FALSE(flag_status(FLAG_CARRY, state.status));
        }

        SECTION("Absolute, X") {
            const Byte opcode = 0x1E;

            SECTION("No Page Cross") {
                const Word addr_effective = (addr_hi << 8) + addr_offset;
                sys.load_program({
                    { 0x8000, {opcode, 0x00, addr_hi} },
                    { addr_effective, {value} }
                });
                sys.processor.registerSet(Mos6502::Register::X, addr_offset);
                sys.memory[addr_effective] = value;
                sys.processor.step();
                auto state = sys.state();
                REQUIRE(state.pc == 0x8003);
                REQUIRE(state.totalcycles == 7);
                REQUIRE(sys.memory[addr_effective] == result);
                REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
                REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
                REQUIRE_FALSE(flag_status(FLAG_CARRY, state.status));
            }

            SECTION("Page Cross") {
                const Word addr_effective = (addr_hi << 8) + 0xFF + addr_offset;
                sys.load_program({
                    { 0x8000, {opcode, 0xFF, addr_hi} },
                    { addr_effective, {value} }
                });
                sys.processor.registerSet(Mos6502::Register::X, addr_offset);
                sys.memory[addr_effective] = value;
                sys.processor.step();
                auto state = sys.state();
                REQUIRE(state.pc == 0x8003);
                REQUIRE(state.totalcycles == 8);
                REQUIRE(sys.memory[addr_effective] == result);
                REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
                REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
                REQUIRE_FALSE(flag_status(FLAG_CARRY, state.status));
            }
        }
    }

    SECTION("Flags") {
        const Byte opcode = 0x0A; // Accumulator
        sys.load_program({
            { 0x8000, {opcode} }
        });

        SECTION("Result Flag Negative/Zero/Carry Clear") {
            const Byte value = 0b00001000;
            const Byte result = 0b00010000;
            sys.processor.registerSet(Mos6502::Register::A, value);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.pc == 0x8001);
            REQUIRE(state.totalcycles == 2);
            REQUIRE(state.a == result);
            REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
            REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
            REQUIRE_FALSE(flag_status(FLAG_CARRY, state.status));
        }

        SECTION("Result Flag Negative Set") {
            const Byte value = 0b01000000;
            const Byte result = 0b10000000;
            sys.processor.registerSet(Mos6502::Register::A, value);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.pc == 0x8001);
            REQUIRE(state.totalcycles == 2);
            REQUIRE(state.a == result);
            REQUIRE(flag_status(FLAG_NEGATIVE, state.status));
            REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
            REQUIRE_FALSE(flag_status(FLAG_CARRY, state.status));
        }

        SECTION("Result Flag Zero Set") {
            const Byte value = 0b00000000;
            const Byte result = 0b00000000;
            sys.processor.registerSet(Mos6502::Register::A, value);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.pc == 0x8001);
            REQUIRE(state.totalcycles == 2);
            REQUIRE(state.a == result);
            REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
            REQUIRE(flag_status(FLAG_ZERO, state.status));
            REQUIRE_FALSE(flag_status(FLAG_CARRY, state.status));
        }

        SECTION("Result Flag Carry Set") {
            const Byte value = 0b10000001;
            const Byte result = 0b00000010;
            sys.processor.registerSet(Mos6502::Register::A, value);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.pc == 0x8001);
            REQUIRE(state.totalcycles == 2);
            REQUIRE(state.a == result);
            REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
            REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
            REQUIRE(flag_status(FLAG_CARRY, state.status));
        }
    }
}

// TODO LSR

TEST_CASE("Instruction ROL", "[Processor][ROL]") {
    CpuTestFixture sys;
    const Byte addr_hi = 0x20;
    const Byte addr_offset = 0x05;
    const Byte value = 0b00100000;
    const Byte result = 0b01000000;

    SECTION("Addressing Modes") {
        SECTION("Accumulator") {
            const Byte opcode = 0x2A;
            sys.load_program({
                { 0x8000, {opcode} }
            });
            sys.processor.registerSet(Mos6502::Register::A, value);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.pc == 0x8001);
            REQUIRE(state.totalcycles == 2);
            REQUIRE(state.a == result);
            REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
            REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
            REQUIRE_FALSE(flag_status(FLAG_CARRY, state.status));
        }

        SECTION("Zero Page") {
            const Byte opcode = 0x26;
            const Word addr_effective = addr_hi;
            sys.load_program({
                { 0x8000, {opcode, addr_hi} },
                { addr_effective, {value} }
            });
            sys.memory[addr_effective] = value;
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 5);
            REQUIRE(sys.memory[addr_effective] == result);
            REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
            REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
            REQUIRE_FALSE(flag_status(FLAG_CARRY, state.status));
        }

        SECTION("Zero Page, X") {
            const Byte opcode = 0x36;
            const Word addr_effective = addr_hi + addr_offset;
            sys.load_program({
                { 0x8000, {opcode, addr_hi} },
                { addr_effective, {value} }
            });
            sys.processor.registerSet(Mos6502::Register::X, addr_offset);
            sys.memory[addr_effective] = value;
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.pc == 0x8002);
            REQUIRE(state.totalcycles == 6);
            REQUIRE(sys.memory[addr_effective] == result);
            REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
            REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
            REQUIRE_FALSE(flag_status(FLAG_CARRY, state.status));
        }

        SECTION("Absolute") {
            const Byte opcode = 0x2E;
            const Word addr_effective = addr_hi << 8;
            sys.load_program({
                { 0x8000, {opcode, addr_effective & 0xFF, addr_effective >> 8} },
                { addr_effective, {value} }
            });
            sys.memory[addr_effective] = value;
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.pc == 0x8003);
            REQUIRE(state.totalcycles == 6);
            REQUIRE(sys.memory[addr_effective] == result);
            REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
            REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
            REQUIRE_FALSE(flag_status(FLAG_CARRY, state.status));
        }

        SECTION("Absolute, X") {
            const Byte opcode = 0x3E;

            SECTION("No Page Cross") {
                const Word addr_effective = (addr_hi << 8) + addr_offset;
                sys.load_program({
                    { 0x8000, {opcode, 0x00, addr_hi} },
                    { addr_effective, {value} }
                });
                sys.processor.registerSet(Mos6502::Register::X, addr_offset);
                sys.memory[addr_effective] = value;
                sys.processor.step();
                auto state = sys.state();
                REQUIRE(state.pc == 0x8003);
                REQUIRE(state.totalcycles == 7);
                REQUIRE(sys.memory[addr_effective] == result);
                REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
                REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
                REQUIRE_FALSE(flag_status(FLAG_CARRY, state.status));
            }

            SECTION("Page Cross") {
                const Word addr_effective = (addr_hi << 8) + 0xFF + addr_offset;
                sys.load_program({
                    { 0x8000, {opcode, 0xFF, addr_hi} },
                    { addr_effective, {value} }
                });
                sys.processor.registerSet(Mos6502::Register::X, addr_offset);
                sys.memory[addr_effective] = value;
                sys.processor.step();
                auto state = sys.state();
                REQUIRE(state.pc == 0x8003);
                REQUIRE(state.totalcycles == 8);
                REQUIRE(sys.memory[addr_effective] == result);
                REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
                REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
                REQUIRE_FALSE(flag_status(FLAG_CARRY, state.status));
            }
        }
    }

    SECTION("Flags") {
        const Byte opcode = 0x2A; // Accumulator
        sys.load_program({
            { 0x8000, {opcode} }
        });

        SECTION("Rotate With Incoming Carry") {
            const Byte value = 0b00001000;
            const Byte result = 0b00010001;
            sys.processor.registerSet(Mos6502::Register::A, value);
            sys.processor.registerSet(Mos6502::Register::Status, FLAG_CARRY);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.pc == 0x8001);
            REQUIRE(state.totalcycles == 2);
            REQUIRE(state.a == result);
            REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
            REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
            REQUIRE_FALSE(flag_status(FLAG_CARRY, state.status));
        }

        SECTION("Result Flag Negative/Zero/Carry Clear") {
            const Byte value = 0b00001000;
            const Byte result = 0b00010000;
            sys.processor.registerSet(Mos6502::Register::A, value);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.pc == 0x8001);
            REQUIRE(state.totalcycles == 2);
            REQUIRE(state.a == result);
            REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
            REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
            REQUIRE_FALSE(flag_status(FLAG_CARRY, state.status));
        }

        SECTION("Result Flag Negative Set") {
            const Byte value = 0b01000000;
            const Byte result = 0b10000000;
            sys.processor.registerSet(Mos6502::Register::A, value);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.pc == 0x8001);
            REQUIRE(state.totalcycles == 2);
            REQUIRE(state.a == result);
            REQUIRE(flag_status(FLAG_NEGATIVE, state.status));
            REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
            REQUIRE_FALSE(flag_status(FLAG_CARRY, state.status));
        }

        SECTION("Result Flag Zero Set") {
            const Byte value = 0b00000000;
            const Byte result = 0b00000000;
            sys.processor.registerSet(Mos6502::Register::A, value);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.pc == 0x8001);
            REQUIRE(state.totalcycles == 2);
            REQUIRE(state.a == result);
            REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
            REQUIRE(flag_status(FLAG_ZERO, state.status));
            REQUIRE_FALSE(flag_status(FLAG_CARRY, state.status));
        }

        SECTION("Result Flag Carry Set") {
            const Byte value = 0b10000001;
            const Byte result = 0b00000010;
            sys.processor.registerSet(Mos6502::Register::A, value);
            sys.processor.step();
            auto state = sys.state();
            REQUIRE(state.pc == 0x8001);
            REQUIRE(state.totalcycles == 2);
            REQUIRE(state.a == result);
            REQUIRE_FALSE(flag_status(FLAG_NEGATIVE, state.status));
            REQUIRE_FALSE(flag_status(FLAG_ZERO, state.status));
            REQUIRE(flag_status(FLAG_CARRY, state.status));
        }
    }
}

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
