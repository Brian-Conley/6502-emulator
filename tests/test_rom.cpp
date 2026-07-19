#include <catch2/catch_test_macros.hpp>
#include <vector>
#include "bytes.hpp"
#include "device.hpp"
#include "rom.hpp"

using namespace Emulator;

TEST_CASE("ROM Initialization", "[ROM]") {
    Byte startPage = 0x10;
    Byte sizeInPages = 0x03;
    std::vector<Byte> data(sizeInPages*256);
    ROM rom(startPage, sizeInPages, data);

    REQUIRE(rom.startPage() == startPage);
    REQUIRE(rom.size() == sizeInPages);
    REQUIRE(rom.type() == DeviceType::ROM);
}

TEST_CASE("ROM Read/Write Operations", "[ROM]") {
    Byte startPage = 0xFD;
    Byte sizeInPages = 0x03;
    std::vector<Byte> data(sizeInPages*256);
    data[0x56] = 0x20;
    data[0x02FF] = 0x21; // Final address
    ROM rom(startPage, sizeInPages, data);

    SECTION("Writing and reading back a valid memory location") {
        Byte targetPage = 0xFD;
        Byte targetOffset = 0x56;
        Byte valueToWrite = 0xAB;

        // Write should do nothing
        rom.write(targetPage, targetOffset, valueToWrite);
        REQUIRE(rom.read(targetPage, targetOffset) == 0x20);
    }

    SECTION("Read final address") {
        REQUIRE(rom.read(startPage+0x02, 0xFF) == 0x21);
    }

}

// Every out of bounds operation should fall back to the ROM's first page.
// Under normal execution, this would never happen, and this is strictly a
//   guard against the misuse of manual read/write calls.
TEST_CASE("ROM out-of-bounds operations", "[ROM]") {
    Byte startPage = 0x80;
    Byte sizeInPages = 0x02;
    std::vector<Byte> data(sizeInPages*256);
    data[0x01] = 0x20;
    ROM rom(startPage, sizeInPages, data);

    SECTION("Read an address higher than bounds") {
        REQUIRE(rom.read(startPage+sizeInPages+0x01, 0x01) == 0x20);
    }
}
