#ifndef ROM_HPP
#define ROM_HPP

#include "bytes.hpp"
#include "device.hpp"
#include <vector>

namespace Emulator {

/**
 * @brief Concrete implementation of Read Only Memory.
 */
class ROM final: public Device {
    public:
        ROM(Byte startPage, Byte size, const std::vector<Byte>& data);

        /**
         * @copydoc Device::read
         */
        Byte read(Byte page, Byte offset) override final;

        /**
         * @copydoc Device::write
         */
        void write(Byte /*page*/, Byte /*offset*/, Byte /*value*/) override;

        ~ROM() override = default;

    private:
        std::vector<Byte> m_memory;
};

}

#endif
