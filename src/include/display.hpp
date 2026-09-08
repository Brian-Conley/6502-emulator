#ifndef DISPLAY_HPP
#define DISPLAY_HPP

#include "bytes.hpp"
#include "device.hpp"
#include "frame.hpp"
#include <vector>

namespace Emulator {

/**
 * @brief Display abstract device
 */
class Display : public Device {
    public:
        enum class RefreshRate {
            ONE,
            TWO,
            FOUR,
            FIVE,
            EIGHT,
            TEN,
            TWENTY,
            TWENTY_FIVE
        };

        /**
         * @brief Get the number of times per second the screen attempts to render a new frame.
         */
        size_t getRefreshRate();

        /**
         * @brief Get the millisecond interval of the refresh rate.
         */
        size_t getRefreshRateInMs();

        /**
         * @copydoc Device::read
         */
        virtual Byte read(Byte page, Byte offset) override = 0;

        /**
         * @copydoc Device::write
         */
        virtual void write(Byte page, Byte offset, Byte value) override = 0;

        /*
         * @brief Convert the buffer into a Frame.
         */
        virtual Frame render() = 0;

        ~Display() override = default;

    protected:
        /**
         * @brief Constructs the Display module with specific page allocation.
         *
         * @param startPage The starting page in the global address space.
         * @param size The total number of pages to allocate.
         * @param width Number of columns in the display.
         * @param height Number of rows in the display.
         * @param refreshRate How many times per second the display should draw a new frame.
         *
         * @note The size of the buffer is width*height.
         *       Creating a buffer larger than the allocated space will result in unusable pixels.
         */
        Display(Byte startPage, Byte size, DeviceType type, size_t width, size_t height, RefreshRate refreshRate);

        size_t m_width; /**< How many columns are in the display. */
        size_t m_height; /**< How many lines are in the display. */
        RefreshRate m_refreshRate; /**< How many times per second to refresh the display. */
        std::vector<Byte> m_buffer; /**< Raw memory buffer of the display. */

    private:
        Display() = delete;
};

}

#endif
