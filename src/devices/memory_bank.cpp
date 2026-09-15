#include "devices/memory_bank.hpp"
#include "devices/device.hpp"
#include <algorithm>

namespace Emulator {

MemoryBank::MemoryBank(Byte startPage, BankSize size, bool isReadOnly, const std::vector<Byte>& initData) :
    Device(startPage, 2, DeviceType::MEMORY_BANK),
    m_isReadOnly(isReadOnly) {

    switch (size) {
        case BankSize::b16: m_pages = std::vector<Byte>(1, 0x00); break;
        case BankSize::b24: m_pages = std::vector<Byte>(2, 0x00); break;
        case BankSize::b32: m_pages = std::vector<Byte>(3, 0x00); break;
    };
    m_memory = std::vector<Byte>(static_cast<size_t>(size) + 1, 0x00);

    if (!initData.empty()) {
        size_t copySize = std::min(initData.size(), m_memory.size());
        std::copy(initData.begin(), initData.begin() + copySize, m_memory.begin());
    }
}

Byte MemoryBank::read(Byte page, Byte offset) {
    if (localPage(page) == 0) {
        return m_pages[offset % m_pages.size()];
    } else {
        return m_memory[localIndex(offset)];
    }
}

void MemoryBank::write(Byte page, Byte offset, Byte value) {
    if (m_isReadOnly) return;
    if (localPage(page) == 0) {
        m_pages[offset % m_pages.size()] = value;
    } else {
        m_memory[localIndex(offset)] = value;
    }
}

size_t MemoryBank::localIndex(Byte offset) {
        uint32_t idx = offset;
        for (size_t i = 0; i < m_pages.size(); ++i) {
            idx |= m_pages[i] << 8*(i+1);
        }
        return idx;
}

}

