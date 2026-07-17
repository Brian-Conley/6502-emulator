#include "rom.hpp"
#include "device.hpp"

namespace Emulator {

ROM::ROM(Byte startPage, Byte size) :
    m_memory(size << 8),
    m_startPage(startPage),
    m_endPage(startPage + size - 1),
    m_size(size)
{}

Byte ROM::read(Byte page, Byte offset) {
    return m_memory[(localPage(page) << 8) | offset];
}

void ROM::write(Byte /*page*/, Byte /*offset*/, Byte /*value*/) { return; }

Byte ROM::startPage() const { return m_startPage; }
Byte ROM::size() const { return m_size; }
DeviceType ROM::type() const { return DeviceType::ROM; }

Byte ROM::localPage(Byte page) {
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
