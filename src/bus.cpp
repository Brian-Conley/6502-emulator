#include "include/bus.hpp"

namespace Emulator {

Bus::Bus(Word memoryCapacity) : memoryCapacity(memoryCapacity) {}

void Bus::registerDevice(std::unique_ptr<Device> device) {
    if (!device) return;
    pageMap[device->page()] = std::move(device);
}

void Bus::write(Word address, Byte value) {
    Byte page = (address >> 8) & 0xFF;

    // Check if a device is assigned to the page.
    if (pageMap[page] != nullptr) {
        // If yes, write to the device register.
        pageMap[page]->write(address & 0xFF, value);
    } else if (address < memoryCapacity) [[likely]] {
        // Write to main memory.
        mainMemory[address] = value;
    }
    // Address does not map to anything, do nothing.
}

Byte Bus::read(Word address) {
    Byte page = (address >> 8) & 0xFF;

    // Check if a device is assigned to the page.
    if (pageMap[page] != nullptr) {
        // If yes, read from device register.
        return pageMap[page]->read(address & 0xFF);
    } else if (address < memoryCapacity) [[likely]] {
        // Read from main memory.
        return mainMemory[address];
    }
    // Address does not map to anything, return a junk sentinel value.
    return 0xFF;
}

}

