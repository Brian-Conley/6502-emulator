#ifndef RAM_HPP
#define RAM_HPP

#include "bytes.hpp"
#include "device.hpp"
#include <vector>

namespace Emulator {

class RAM : public Device {
    public:
        RAM(Byte startPage, Byte size);

        Byte read(Byte page, Byte offset) override final;
        void write(Byte page, Byte offset, Byte value) override final;
        Byte startPage() const override final;
        Byte size() const override final; /**< Number of pages */
        DeviceType type() const override final;

        ~RAM() override = default;

    private:
        Byte localPage(Byte page);
        std::vector<Byte> m_memory;
        const Byte m_startPage;
        const Byte m_endPage;
        const Byte m_size;
};

}

#endif
