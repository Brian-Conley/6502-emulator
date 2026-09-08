#include "display.hpp"
#include "device.hpp"

namespace Emulator {

size_t Display::getRefreshRate() {
    switch(m_refreshRate) {
        case RefreshRate::ONE: return 1;
        case RefreshRate::TWO: return 2;
        case RefreshRate::FOUR: return 4;
        case RefreshRate::FIVE: return 5;
        case RefreshRate::EIGHT: return 8;
        case RefreshRate::TEN: return 10;
        case RefreshRate::TWENTY: return 20;
        case RefreshRate::TWENTY_FIVE: return 25;
    }
    return 1; /**< Should be unreachable. This is to satisfy a compiler warning regarding return values at the end of function. */
}

size_t Display::getRefreshRateInMs() {
    switch(m_refreshRate) {
        case RefreshRate::ONE: return 1000;
        case RefreshRate::TWO: return 500;
        case RefreshRate::FOUR: return 250;
        case RefreshRate::FIVE: return 200;
        case RefreshRate::EIGHT: return 125;
        case RefreshRate::TEN: return 100;
        case RefreshRate::TWENTY: return 50;
        case RefreshRate::TWENTY_FIVE: return 40;
    }
    return 1000; /**< Should be unreachable. This is to satisfy a compiler warning regarding return values at the end of function. */
}

Display::Display(Byte startPage, Byte size, DeviceType type, size_t width, size_t height, RefreshRate refreshRate) :
    Device(startPage, size, type),
    m_width(width),
    m_height(height),
    m_refreshRate(refreshRate),
    m_buffer(256*size, 0x00)
{}

}
