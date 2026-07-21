#include "device.hpp"

namespace Emulator {

Device::Device(Byte startPage, Byte size, DeviceType type) :
    deviceConfig{
        startPage,
        static_cast<Byte>(startPage + size - 1),
        size,
        type
    }
{}

Byte Device::startPage() const { return deviceConfig.startPage; }

Byte Device::size() const { return deviceConfig.size; }

DeviceType Device::type() const { return deviceConfig.type; }

Byte Device::localPage(Byte page) const {
    // This bounds check is completely redundant under normal execution.
    // The bus should always assign valid pages and call read/write with valid addresses.
    // But, manual calls outside the bus might provide invalid parameters,
    //   so this will just default to page 0 in such cases.
    if (page < deviceConfig.startPage || page > deviceConfig.endPage)
        return 0x00;
    return page - deviceConfig.startPage;
}

}
