#include <catch2/catch_test_macros.hpp>
#include <memory>
#include "bus.hpp"

// Used for dummy devices to test page registering and read/write operations
#include "devices/memory.hpp"

using namespace Emulator;

TEST_CASE("Registering Device objects", "[Bus]") {
    Bus bus;
    auto ram1 = std::make_shared<Memory>(0x00, 0x80);
    auto ram2 = std::make_shared<Memory>(0x80, 0x80);
    auto ram3 = std::make_shared<Memory>(0x10, 0x05);

    SECTION("Register every page in the address space with two devices") {
        REQUIRE(bus.registerDevice(ram1));
        REQUIRE(bus.registerDevice(ram2));
    }

    SECTION("Attempt to register a device to pages that are already registered") {
        REQUIRE(bus.registerDevice(ram1));
        REQUIRE_FALSE(bus.registerDevice(ram3));
    }
}

TEST_CASE("Read and write to addresses", "[Bus]") {
    Bus bus;
    auto ram = std::make_shared<Memory>(0x00, 1);

    SECTION("Read and write to a registered address") {
        bus.registerDevice(ram);

        ram->write(0x00, 0x20, 0x45);
        REQUIRE(bus.read(0x0020) == 0x45);

        bus.write(0x0020, 0x80);
        REQUIRE(ram->read(0x00, 0x20) == 0x80);
    }

    SECTION("Read and write to an unregistered address") {
        REQUIRE(bus.read(0x0101) == 0xFF); // 0xFF is a current placeholder value for open bus behavior
        bus.write(0x0101, 0x45); // Should do nothing
        REQUIRE(bus.read(0x0101) == 0xFF); // Expect the same placeholder value
    }
}
