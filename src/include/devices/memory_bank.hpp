#ifndef MEMORY_BANK_HPP
#define MEMORY_BANK_HPP

#include "bytes.hpp"
#include "device.hpp"
#include <vector>

namespace Emulator {

/**
 * @brief Concrete implementation of MemoryBank.
 */
class MemoryBank final : public Device {
    public:
        enum class BankSize : size_t {
            b16 = 0xFFFF,    // 16-bit address space
            b24 = 0xFFFFFF,  // 24-bit address space
            b32 = 0xFFFFFFFF // 32-bit address space
        };

        /**
         * @brief Constructs the Memory module with specific page allocation.
         *
         * @param startPage The starting page in the global address space.
         * @param size Preset capacity of the memory bank.
         * @param isReadOnly True for ROM, false for RAM.
         * @param initData Initialize the internal buffer to the provided data.
         */
        MemoryBank(Byte startPage, BankSize size, bool isReadOnly = false, const std::vector<Byte>& initData = {});

        /**
         * @copydoc Device::read
         */
        Byte read(Byte page, Byte offset) override;

        /**
         * @copydoc Device::write
         */
        void write(Byte page, Byte offset, Byte value) override;

        ~MemoryBank() override = default;

    private:
        size_t localIndex(Byte offset); /**< Converts the offset into the real memory index based on the page vector. */

        std::vector<Byte> m_pages; /**< Pages are in little endian. */
        std::vector<Byte> m_memory;
        bool m_isReadOnly;
};

}

#endif
