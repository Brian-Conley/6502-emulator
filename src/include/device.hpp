#ifndef DEVICE_HPP
#define DEVICE_HPP

#include "bytes.hpp"

namespace Emulator {

class Device {
    public:
        virtual Byte read(Byte address) = 0;
        virtual void write(Byte address, Byte value) = 0;
        virtual Byte page() const = 0;

        virtual ~Device() = default;
};

}

#endif
