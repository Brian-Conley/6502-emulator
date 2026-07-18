#include <catch2/catch_test_macros.hpp>
#include "bytes.hpp"
#include "device.hpp"
#include "ram.hpp"

using namespace Emulator;

TEST_CASE("RAM Initialization", "[RAM]") {
    Byte startPage = 0x10;
    Byte sizeInPages = 0x03;
    RAM ram(startPage, sizeInPages);

    REQUIRE(ram.startPage() == startPage);
    REQUIRE(ram.size() == sizeInPages);
    REQUIRE(ram.type() == DeviceType::RAM);
}

TEST_CASE("RAM Read/Write Operations", "[RAM]") {
    Byte startPage = 0xFD;
    Byte sizeInPages = 0x03;
    RAM ram(startPage, sizeInPages);

    SECTION("Reading an uninitialized address should return default value. (0x00)") {
        REQUIRE(ram.read(startPage, 0x05) == 0x00);
    }

    SECTION("Writing and reading back a valid memory location") {
        Byte targetPage = 0xFD;
        Byte targetOffset = 0x56;
        Byte valueToWrite = 0xAB;

        ram.write(targetPage, targetOffset, valueToWrite);
        REQUIRE(ram.read(targetPage, targetOffset) == valueToWrite);
    }

    SECTION("Multiple independent memory locations do not overwrite each other") {
        ram.write(0xFD, 0xFF, 0x01);
        ram.write(0xFE, 0xFF, 0x02);

        REQUIRE(ram.read(0xFD, 0xFF) == 0x01);
        REQUIRE(ram.read(0xFE, 0xFF) == 0x02);
    }

    SECTION("Write to final address") {
        ram.write(0xFF, 0xFF, 0x20);
        REQUIRE(ram.read(0xFF, 0xFF) == 0x20);
    }

}

// Every out of bounds operation should fall back to the RAM's first page.
// Under normal execution, this would never happen, and this is strictly a
//   guard against the misuse of manual read/write calls.
TEST_CASE("RAM out-of-bounds operations", "[RAM]") {
    Byte startPage = 0x80;
    Byte sizeInPages = 0x02;
    RAM ram(startPage, sizeInPages);

    SECTION("Write to address lower than bounds") {
        ram.write(startPage-0x01, 0x05, 0x20);
        REQUIRE(ram.read(startPage, 0x05) == 0x20);
    }

    SECTION("Write to address higher than bounds") {
        ram.write(startPage+sizeInPages+0x01, 0x50, 0x20);
        REQUIRE(ram.read(startPage, 0x50) == 0x20);
    }
}
