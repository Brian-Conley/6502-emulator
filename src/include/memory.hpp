#ifndef MEMORY_HPP
#define MEMORY_HPP

#include "bytes.hpp"
#include "device.hpp"
#include <vector>

namespace Emulator {

/**
 * @brief Concrete implementation of Memory.
 */
class Memory final : public Device {
    public:
        /**
         * @brief Constructs the Memory module with specific page allocation.
         *
         * @param startPage The starting page in the global address space.
         * @param size The total number of pages to allocate.
         * @param isReadOnly True for ROM, false for RAM.
         * @param initData Initialize the internal buffer to the provided data.
         */
        Memory(Byte startPage, Byte size, bool isReadOnly = false, const std::vector<Byte>& initData = {});

        /**
         * @copydoc Device::read
         */
        Byte read(Byte page, Byte offset) override;

        /**
         * @copydoc Device::write
         */
        void write(Byte page, Byte offset, Byte value) override;

        ~Memory() override = default;

    private:
        std::vector<Byte> m_memory;
        bool m_isReadOnly;
};

}

#endif
