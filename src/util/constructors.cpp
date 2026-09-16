#include "util/constructors.hpp"
#include "devices/memory.hpp"
#include "devices/ascii_display.hpp"
#include "devices/memory_bank.hpp"
#include "devices/pixel_display.hpp"

namespace Emulator {
namespace Constructors {

namespace {
    Byte calculateDisplaySize(size_t width, size_t height) {
        size_t area = width * height;
        return (area >> 8) + (area % 256 == 0 ? 0 : 1);
    }
}

std::shared_ptr<Device> createRomDevice(Byte startPage, Byte size, const std::vector<Byte>& initData) {
    return std::make_shared<Memory>(startPage, size, true, initData);
}

std::shared_ptr<Device> createRamDevice(Byte startPage, Byte size, const std::vector<Byte>& initData) {
    return std::make_shared<Memory>(startPage, size, false, initData);
}

std::shared_ptr<Device> create16BitRomBankDevice(Byte startPage, const std::vector<Byte>& initData) {
    return std::make_shared<MemoryBank>(startPage, MemoryBank::BankSize::b16, true, initData);
}

std::shared_ptr<Device> create24BitRomBankDevice(Byte startPage, const std::vector<Byte>& initData) {
    return std::make_shared<MemoryBank>(startPage, MemoryBank::BankSize::b24, true, initData);
}

std::shared_ptr<Device> create32BitRomBankDevice(Byte startPage, const std::vector<Byte>& initData) {
    return std::make_shared<MemoryBank>(startPage, MemoryBank::BankSize::b32, true, initData);
}

std::shared_ptr<Device> create16BitRamBankDevice(Byte startPage, const std::vector<Byte>& initData) {
    return std::make_shared<MemoryBank>(startPage, MemoryBank::BankSize::b16, false, initData);
}

std::shared_ptr<Device> create24BitRamBankDevice(Byte startPage, const std::vector<Byte>& initData) {
    return std::make_shared<MemoryBank>(startPage, MemoryBank::BankSize::b24, false, initData);
}

std::shared_ptr<Device> create32BitRamBankDevice(Byte startPage, const std::vector<Byte>& initData) {
    return std::make_shared<MemoryBank>(startPage, MemoryBank::BankSize::b32, false, initData);
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
