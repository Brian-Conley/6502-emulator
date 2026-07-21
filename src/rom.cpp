#include "rom.hpp"
#include "bytes.hpp"
#include "device.hpp"
#include <algorithm>

namespace Emulator {

ROM::ROM(Byte startPage, Byte size, const std::vector<Byte>& data) :
    Device(startPage, size, DeviceType::ROM),
    m_memory(size << 8)
{
    size_t bytesToCopy = std::min(data.size(), m_memory.size());
    std::copy_n(data.begin(), bytesToCopy, m_memory.begin());
}

Byte ROM::read(Byte page, Byte offset) {
    return m_memory[(localPage(page) << 8) | offset];
}

void ROM::write(Byte /*page*/, Byte /*offset*/, Byte /*value*/) { return; }

}
