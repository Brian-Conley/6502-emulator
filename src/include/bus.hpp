#ifndef BUS_HPP
#define BUS_HPP

#include "bytes.hpp"
#include "device.hpp"
#include <array>

namespace Emulator {

class Bus {
    public:
        Bus();

        void registerDevice(Device* device);
        void write(Word address, Byte value);
        Byte read(Word address);

    private:
        std::array<Device*, 256> pageMap{};
};

}

#endif
