#ifndef BUS_HPP
#define BUS_HPP

#include "bytes.hpp"
#include "device.hpp"
#include <array>
#include <memory>

namespace Emulator {

class Bus {
    public:
        Bus(Word memoryCapacity);

        void registerDevice(std::unique_ptr<Device> device);
        void write(Word address, Byte value);
        Byte read(Word address);

    private:
        Word memoryCapacity;

        // The real main memory of the system.
        std::array<Byte, 65536> mainMemory{};

        // Map of page number to registered Device pointer (nullptr means raw memory).
        std::array<std::unique_ptr<Device>, 256> pageMap{};

        Bus() = delete;
};

}

#endif
