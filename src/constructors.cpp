#include "constructors.hpp"
#include "memory.hpp"
#include "ascii_display.hpp"
#include "pixel_display.hpp"

namespace Emulator {
namespace Constructors {

namespace {
    Byte calculateDisplaySize(size_t width, size_t height) {
        size_t area = width * height;
        return (area >> 8) + (area % 256 == 0 ? 0 : 1);
    }
}

std::shared_ptr<Device> createMemoryDevice(Byte startPage, Byte size, bool readOnly, const std::vector<Byte>& initData) {
    return std::make_shared<Memory>(startPage, size, readOnly, initData);
}

std::shared_ptr<Display> createAsciiDisplayDevice(
        Byte startPage,
        size_t width,
        size_t height,
        Display::RefreshRate refreshRate) {

    return std::make_shared<AsciiDisplay>(startPage, calculateDisplaySize(width, height), width, height, refreshRate);
}

std::shared_ptr<Display> createPixelDisplayDevice(
        Byte startPage,
        size_t width,
        size_t height,
        Display::RefreshRate refreshRate) {

    return std::make_shared<PixelDisplay>(startPage, calculateDisplaySize(width, height), width, height, refreshRate);
}

std::shared_ptr<SystemInterface> createSystemInterfaceDevice(Byte startPage) {
    return std::make_shared<SystemInterface>(startPage);
}

}}
