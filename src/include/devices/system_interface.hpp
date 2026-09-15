#ifndef SYSTEM_INTERFACE_HPP
#define SYSTEM_INTERFACE_HPP

#include "bytes.hpp"
#include "device.hpp"

namespace Emulator {

/**
 * @brief Concrete implementation of SystemInterface.
 */
class SystemInterface final : public Device {
    public:
        /**
         * @brief Constructs the SystemInterface module with specific page allocation.
         *
         * @param startPage The starting page in the global address space.
         */
        SystemInterface(Byte startPage);

        /**
         * @copydoc Device::read
         */
        Byte read(Byte page, Byte offset) override;

        /**
         * @copydoc Device::write
         */
        void write(Byte page, Byte offset, Byte value) override;

        ~SystemInterface() override = default;

    private:
        SystemInterface() = delete;

        const Byte m_commands = 0x02;
};

}

#endif
