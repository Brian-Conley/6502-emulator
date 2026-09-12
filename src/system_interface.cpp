#include "system_interface.hpp"

namespace Emulator {

SystemInterface::SystemInterface(Byte startPage) :
    Device(startPage, 0x01, DeviceType::SYSTEM_INTERFACE)
{}

Byte SystemInterface::read(Byte page, Byte offset) { return (page << 8) | offset; }

void SystemInterface::write(Byte page, Byte offset, Byte value) {
    if (!m_powered)
        return;
    if (value == 0x00)
        return;

    Word address = (localPage(page) << 8) | offset;

    // Power off
    if (address % m_commands == 0x00) {
        powerOff();
        return;
    }

    // Reset
    if (address % m_commands == 0x01) {
        reset();
        return;
    }
}

}
