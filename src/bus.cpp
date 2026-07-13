#include "include/bus.hpp"

namespace Emulator {

Bus::Bus() {}

void Bus::registerDevice(Device* device) {
    if (!device) return;
    Byte startPage = device->startPage();
    Byte size = device->size();
    Word endPage = startPage + size - 1;
    if (size == 0 || endPage > 0xFF) return;

    for (int i = startPage; i < (startPage + size); ++i)
        pageMap[i] = device;
}

void Bus::write(Word address, Byte value) {
    Byte page = (address >> 8) & 0xFF;
    Byte offset = address & 0xFF;

    // Check if a device is assigned to the page.
    if (pageMap[page] != nullptr) {
        // If yes, write to the device register.
        pageMap[page]->write(page, offset, value);
    }
    // Address does not map to anything, do nothing.
}

Byte Bus::read(Word address) {
    Byte page = (address >> 8) & 0xFF;
    Byte offset = address & 0xFF;

    // Check if a device is assigned to the page.
    if (pageMap[page] != nullptr) [[likely]] {
        // If yes, read from device register.
        return pageMap[page]->read(page, offset);
    }
    // Address does not map to anything, return a junk sentinel value.
    return 0xFF;
}

}

