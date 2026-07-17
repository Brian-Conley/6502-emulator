#ifndef BUS_HPP
#define BUS_HPP

#include "bytes.hpp"
#include "device.hpp"
#include <array>
#include <memory>

namespace Emulator {

class Bus {
    public:
        Bus();

        void registerDevice(std::shared_ptr<Device> device);
        void write(Word address, Byte value);
        Byte read(Word address);

    private:
        std::array<std::shared_ptr<Device>, 256> pageMap{};
};

}

#endif
