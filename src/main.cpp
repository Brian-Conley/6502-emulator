#include "bus.hpp"
#include "ram.hpp"
#include <iostream>
#include <memory>

int main() {
    Emulator::Bus bus;
    auto ram = std::make_shared<Emulator::RAM>(0x00, 0x20);
    bus.registerDevice(ram);

    bus.write(0x1040, 0x41);
    std::cout << bus.read(0x1040) << '\n';
    std::cout << ram->read(0x10, 0x40) << '\n';

}
