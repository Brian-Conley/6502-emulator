#ifndef CONSTRUCTORS_HPP
#define CONSTRUCTORS_HPP

#include "bytes.hpp"
#include "device.hpp"
#include "display.hpp"
#include <memory>

namespace Emulator {
namespace Constructors {

/**
 * @brief Create a Memory object.
 *
 * @param startPage The starting page in the global address space.
 * @param size The total number of pages to allocate.
 * @param readOnly True for ROM, false for RAM.
 *
 * @return Device smart pointer of the created object.
 */
std::shared_ptr<Device> createMemoryDevice(Byte startPage, Byte size, bool readOnly = false, const std::vector<Byte>& initData = {});

/**
 * @brief Create an ascii Display object (for a terminal environment).
 *
 * @param startPage The starting page in the global address space.
 * @param width Number of pixel columns.
 * @param height Number of pixel rows.
 * @param refreshRate How many times per second the display refreshes. (Display::RefreshRate)
 *
 * @return Display smart pointer of the created object.
 */
std::shared_ptr<Display> createAsciiDisplayDevice(
        Byte startPage,
        size_t width,
        size_t height,
        Display::RefreshRate refreshRate);


/**
 * @brief Create a pixel Display object (for a terminal environment).
 *
 * @param startPage The starting page in the global address space.
 * @param width Number of pixel columns.
 * @param height Number of pixel rows.
 * @param refreshRate How many times per second the display refreshes. (Display::RefreshRate)
 *
 * @return Display smart pointer of the created object.
 */
std::shared_ptr<Display> createPixelDisplayDevice(
        Byte startPage,
        size_t width,
        size_t height,
        Display::RefreshRate refreshRate);

}}

#endif
