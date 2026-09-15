#ifndef DEVICE_HPP
#define DEVICE_HPP

#include "bytes.hpp"
#include <functional>

namespace Emulator {

enum class DeviceType {
    RAM,
    ROM,
    DISPLAY_ASCII,
    DISPLAY_PIXEL,
    SYSTEM_INTERFACE,
    MEMORY_BANK
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
        void powerOff();             /**< Call the powerOff callback if bound. */
        void reset();                /**< Call the reset callback if bound. Reset referrs to triggering a processor reset in this case. */
        void irq();                  /**< Call the IRQ callback if bound. */
        void bindPowerOff(std::function<void()> callback); /**< Bind a callback function to powerOff. Devices execute the callback by default, but derived classes may have different implementations. */
        void bindIRQ(std::function<void()> callback);      /**< Bind a callback function to irq(). */
        void bindReset(std::function<void()> callback);    /**< Bind a callback function to reset(). */

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

        std::function<void()> m_powerOffCallback;
        std::function<void()> m_resetCallback;
        std::function<void()> m_irqCallback;
};

}

#endif
