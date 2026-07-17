#include "ram.hpp"
#include "device.hpp"

namespace Emulator {

RAM::RAM(Byte startPage, Byte size) :
    m_memory(size << 8),
    m_startPage(startPage),
    m_endPage(startPage + size),
    m_size(size)
{}

Byte RAM::read(Byte page, Byte offset) {
    return m_memory[(localPage(page) << 8) | offset];
}

void RAM::write(Byte page, Byte offset, Byte value) {
    m_memory[(localPage(page) << 8) | offset] = value;
}

Byte RAM::startPage() const { return m_startPage; }
Byte RAM::size() const { return m_size; }
DeviceType RAM::type() const { return DeviceType::RAM; }

Byte RAM::localPage(Byte page) {
    // This bounds check is completely redundant under normal execution.
    // The bus should always assign valid pages and call read/write with valid addresses.
    // But, manual calls outside the bus might provide invalid parameters,
    //   so this will just default to page 0 in such cases.
    int lp = page - m_startPage;
    if (lp < 0 || lp > m_endPage)
        return 0x00;
    return lp;
}

}
