#include "device.hpp"

namespace Emulator {

Device::Device(Byte startPage, Byte size, DeviceType type) :
    deviceConfig{
        startPage,
        static_cast<Byte>(startPage + size - 1),
        size,
        type
    },
    m_powered(false),
    m_mutex()
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

bool Device::powerStatus() const { return m_powered; }

void Device::powerOn() { m_powered = true; }

void Device::powerOff() {
    if (!m_powered)
        return;
    m_powered = false;
    if (m_powerOffCallback)
        m_powerOffCallback();
}

void Device::reset() {
    if (m_resetCallback)
        m_resetCallback();
}

void Device::irq() {
    if (m_irqCallback)
        m_irqCallback();
}

void Device::bindPowerOff(std::function<void()> callback) { m_powerOffCallback = callback; }

void Device::bindIRQ(std::function<void()> callback) { m_irqCallback = callback; }

void Device::bindReset(std::function<void()> callback) { m_resetCallback = callback; }

}
