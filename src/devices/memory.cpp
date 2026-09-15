#include "devices/memory.hpp"
#include <algorithm>

namespace Emulator {

Memory::Memory(Byte startPage, Byte size, bool isReadOnly, const std::vector<Byte>& initData) :
    Device(startPage, size, isReadOnly ? DeviceType::ROM : DeviceType::RAM),
    m_memory(size << 8, 0x00),
    m_isReadOnly(isReadOnly) {

    if (!initData.empty()) {
        size_t copySize = std::min(initData.size(), m_memory.size());
        std::copy(initData.begin(), initData.begin() + copySize, m_memory.begin());
    }
}

Byte Memory::read(Byte page, Byte offset) {
    return m_memory[(localPage(page) << 8) | offset];
}

void Memory::write(Byte page, Byte offset, Byte value) {
    if (m_isReadOnly) return;
    m_memory[(localPage(page) << 8) | offset] = value;
}

}
