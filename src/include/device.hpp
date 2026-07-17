#ifndef DEVICE_HPP
#define DEVICE_HPP

#include "bytes.hpp"

namespace Emulator {

enum class DeviceType {
    RAM,
};

class Device {
    public:
        virtual Byte read(Byte page, Byte offset) = 0;
        virtual void write(Byte page, Byte offset, Byte value) = 0;
        virtual Byte startPage() const = 0;
        virtual Byte size() const = 0; /**< Number of pages */
        virtual DeviceType type() const = 0;

        virtual ~Device() = default;
};

}

#endif
