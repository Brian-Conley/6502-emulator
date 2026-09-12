#include "pixel_display.hpp"
#include "frame.hpp"
#include "bus.hpp"

namespace Emulator {

PixelDisplay::PixelDisplay(Byte startPage, Byte size, size_t width, size_t height, Display::RefreshRate refreshRate) :
    Display(startPage, size, DeviceType::DISPLAY_PIXEL, width, height, refreshRate)
{}

Frame PixelDisplay::render() {
    for (size_t i = 0; i < std::min(m_buffer.size(), m_frameData.size()); ++i) {
        Byte buffVal = m_buffer[i];
        Byte colorIdx = buffVal >> 4;
        Byte glyphIdx = buffVal & 0b00001111;

        Color color;
        char glyph;
        switch(colorIdx) {
            case 0b0000: color = Color::white; break;
            case 0b0001: color = Color::red; break;
            case 0b0010: color = Color::green; break;
            case 0b0011: color = Color::blue; break;
            case 0b0100: color = Color::yellow; break;
            case 0b0101: color = Color::magenta; break;
            case 0b0110: color = Color::cyan; break;
            case 0b0111: color = Color::black; break;
            case 0b1000: color = Color::bright_white; break;
            case 0b1001: color = Color::bright_red; break;
            case 0b1010: color = Color::bright_green; break;
            case 0b1011: color = Color::bright_blue; break;
            case 0b1100: color = Color::bright_yellow; break;
            case 0b1101: color = Color::bright_magenta; break;
            case 0b1110: color = Color::bright_cyan; break;
            case 0b1111: color = Color::bright_black; break;
            default: color = Color::reset;
        };

        switch(glyphIdx) {
            case 0b0000: glyph = ' '; break;
            case 0b0001: glyph = '.'; break;
            case 0b0010: glyph = ','; break;
            case 0b0011: glyph = '-'; break;
            case 0b0100: glyph = '~'; break;
            case 0b0101: glyph = ':'; break;
            case 0b0110: glyph = ';'; break;
            case 0b0111: glyph = '='; break;
            case 0b1000: glyph = '+'; break;
            case 0b1001: glyph = '*'; break;
            case 0b1010: glyph = '#'; break;
            case 0b1011: glyph = '&'; break;
            case 0b1100: glyph = '$'; break;
            case 0b1101: glyph = '@'; break;
            case 0b1110: glyph = 'M'; break;
            case 0b1111: glyph = 'W'; break;
            default: glyph = ' ';
        };

        m_frameData[i].color = color;
        m_frameData[i].glyph = glyph;
    }

    if (m_irqCallback)
        m_irqCallback();
    return { m_width, m_height, m_frameData };
}

}

