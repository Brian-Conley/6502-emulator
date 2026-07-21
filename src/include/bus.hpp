#ifndef BUS_HPP
#define BUS_HPP

#include "bytes.hpp"
#include "device.hpp"
#include <array>
#include <memory>

namespace Emulator {

/**
 * @brief The medium by which the processor interacts with its address space.
 *
 * Provides a public interface to interact with the processor's address space
 * using read and write operations.
 * The address space is split into 256 units of pages (1 page = 256 bytes).
 */
class Bus {
    public:
        Bus();

        /**
         * @brief Allocate pages to the given Device object.
         *        Allocated pages are defined by the device.
         *
         * @param device The target Device object for page allocations.
         *
         * @note Registering a device can overwrite existing page allocations.
         */
        void registerDevice(std::shared_ptr<Device> device);

        /**
         * @brief Write a value to the specified address.
         *
         * @param address The target memory address to write to.
         * @param value The byte value to be written.
         *
         * @note Writing to an unregistered address will exhibit open-bus behavior.
         */
        void write(Word address, Byte value);

        /**
         * @brief Read a value from the specified address.
         *
         * @param address The target memory address to read from.
         * @return Byte The value read from the address.
         *
         * @note Reading from an unregistered address will exhibit open-bus behavior.
         */
        Byte read(Word address);

    private:
        std::array<std::shared_ptr<Device>, 256> pageMap{}; /**< The map of registered pages for each Device. */
};

}

#endif
