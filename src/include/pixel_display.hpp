#ifndef PIXEL_DISPLAY_HPP
#define PIXEL_DISPLAY_HPP

#include "bytes.hpp"
#include "display.hpp"
#include "frame.hpp"
#include <vector>

namespace Emulator {

/**
 * @brief Display that renders colored pixel frames.
 */
class PixelDisplay final : public Display {
    public:

        /**
         * @brief Constructs the PixelDisplay module with specific page allocation.
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
        PixelDisplay(Byte startPage, Byte size, size_t width, size_t height, Display::RefreshRate refreshRate);

        /**
         * @copydoc Device::read
         */
        Byte read(Byte page, Byte offset) override;

        /**
         * @copydoc Device::write
         */
        void write(Byte page, Byte offset, Byte value) override;

        /*
         * @brief Convert the buffer into a Frame.
         */
        Frame render() override;

        ~PixelDisplay() override = default;

    private:
        PixelDisplay() = delete;

        std::vector<Pixel> m_frameData; /**< Frame buffer. */
};

}

#endif
