#include "devices/ascii_display.hpp"
#include <algorithm>

namespace Emulator {

AsciiDisplay::AsciiDisplay(Byte startPage, Byte size, size_t width, size_t height, Display::RefreshRate refreshRate) :
    Display(startPage, size, DeviceType::DISPLAY_ASCII, width, height, refreshRate)
{}

Frame AsciiDisplay::render() {
    // If the byte is a printable ascii char, write it to the frame buffer.
    // If it's not printable, write a space instead.
    for (size_t i = 0; i < std::min(m_buffer.size(), m_frameData.size()); ++i) {
        Byte g = m_buffer[i];
        m_frameData[i].glyph = (g >= 0x20 && g < 0x7F) ? g : 0x20;
    }

    if (m_irqCallback)
        m_irqCallback();

    return { m_width, m_height, m_frameData };
}

};

