#ifndef RAM_HPP
#define RAM_HPP

#include "bytes.hpp"
#include "device.hpp"
#include <vector>

namespace Emulator {

/**
 * @brief Concrete implementation of Random Access Memory.
 */
class RAM final : public Device {
    public:
        /**
         * @brief Constructs the RAM module with specific page allocation.
         *
         * @param startPage The starting page in the global address space.
         * @param size The total number of pages to allocate.
         */
        RAM(Byte startPage, Byte size);

        /**
         * @copydoc Device::read
         */
        Byte read(Byte page, Byte offset) override;

        /**
         * @copydoc Device::write
         */
        void write(Byte page, Byte offset, Byte value) override;

        ~RAM() override = default;

    private:
        std::vector<Byte> m_memory;
};

}

#endif
