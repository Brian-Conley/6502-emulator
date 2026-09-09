#ifndef DEVICE_HPP
#define DEVICE_HPP

#include "bytes.hpp"
#include <atomic>

namespace Emulator {

class Bus;

enum class DeviceType {
    RAM,
    ROM,
    DISPLAY_ASCII,
    DISPLAY_PIXEL
};

/**
 * @brief Abstract class defining the public interface for every Device implementation.
 */
class Device {
    public:
        /**
         * @brief Read a byte from the device.
         *
         * @param page The high byte of the address.
         * @param offset The low byte of the offset.
         *
         * @return The byte value at the provided address.
         */
        virtual Byte read(Byte page, Byte offset) = 0;

        /**
         * @brief Write a byte to the device's memory.
         *
         * @param page The high byte of the address.
         * @param offset The low byte of the address.
         */
        virtual void write(Byte page, Byte offset, Byte value) = 0;

        Byte startPage() const;      /**< First page occupied in the global address space. */
        Byte size() const;           /**< Number of pages allocated to the object. */
        DeviceType type() const;     /**< Category of the device. */
        void setBus(Bus* systemBus); /**< Register the system bus to this device. */
        virtual void powerOn();      /**< Turn the device on. */
        virtual void powerOff();     /**< Turn the device off. */
        bool powerStatus() const;    /**< Get the power status of the device. */

        virtual ~Device() = default;

    protected:
        struct DeviceConfig {
            const Byte startPage;
            const Byte endPage;
            const Byte size;
            const DeviceType type;
        };

        Device(Byte startPage, Byte size, DeviceType type);
        const DeviceConfig deviceConfig;
        Byte localPage(Byte page) const;

        std::atomic<bool> m_powered; /**< Used to determine if a device's thread should continue execution. */
        Bus* m_systemBus;            /**< Reference to the system bus for IRQ signals. */
};

}

#endif
