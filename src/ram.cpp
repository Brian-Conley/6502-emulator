#include "ram.hpp"
#include "device.hpp"

namespace Emulator {

RAM::RAM(Byte startPage, Byte size) :
    Device(startPage, size, DeviceType::RAM),
    m_memory(size << 8)
{}

Byte RAM::read(Byte page, Byte offset) {
    return m_memory[(localPage(page) << 8) | offset];
}

void RAM::write(Byte page, Byte offset, Byte value) {
    m_memory[(localPage(page) << 8) | offset] = value;
}

}
