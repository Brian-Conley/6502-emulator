#include "ascii_display.hpp"
#include "bus.hpp"
#include <algorithm>

namespace Emulator {

AsciiDisplay::AsciiDisplay(Byte startPage, Byte size, size_t width, size_t height, Display::RefreshRate refreshRate) :
    Display(startPage, size, DeviceType::DISPLAY_ASCII, width, height, refreshRate)
{}

Byte AsciiDisplay::read(Byte page, Byte offset) {
    return m_buffer[(localPage(page) << 8) | offset];
}

void AsciiDisplay::write(Byte page, Byte offset, Byte value) {
    m_buffer[(localPage(page) << 8) | offset] = value;
}

Frame AsciiDisplay::render() {
    for (size_t i = 0; i < std::min(m_buffer.size(), m_frameData.size()); ++i) {
        Byte g = m_buffer[i];
        m_frameData[i].glyph = (g >= 0x20 && g < 0x7F) ? g : 0x20;
    }

    // Trigger the interrupt routine to prepare the next frame.
    if (m_systemBus) m_systemBus->setIRQ();
    return { m_width, m_height, m_frameData };
}

};

