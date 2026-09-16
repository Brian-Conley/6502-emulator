#ifndef DEVICE_FACTORIES_HPP
#define DEVICE_FACTORIES_HPP

#include "bytes.hpp"
#include "devices/device.hpp"
#include "devices/display.hpp"
#include "devices/system_interface.hpp"
#include <functional>
#include <memory>

namespace Emulator {
namespace DeviceFactory {

/**
 * @brief Create a ROM object.
 *
 * @param startPage The starting page in the global address space.
 * @param size The total number of pages to allocate.
 * @param initData Initialize the contents to this data
 *
 * @return Device smart pointer of the created object.
 */
std::shared_ptr<Device> createRomDevice(Byte startPage, Byte size, const std::vector<Byte>& initData = {});

/**
 * @brief Create a RAM object.
 *
 * @param startPage The starting page in the global address space.
 * @param size The total number of pages to allocate.
 * @param initData Initialize the contents to this data
 *
 * @return Device smart pointer of the created object.
 */
std::shared_ptr<Device> createRamDevice(Byte startPage, Byte size, const std::vector<Byte>& initData = {});

/**
 * @brief Create a 16-Bit ROM Bank object.
 *
 * @param startPage The starting page in the global address space.
 * @param initData Initialize the contents to this data
 *
 * @return Device smart pointer of the created object.
 */
std::shared_ptr<Device> create16BitRomBankDevice(Byte startPage, const std::vector<Byte>& initData = {});

/**
 * @brief Create a 24-Bit ROM Bank object.
 *
 * @param startPage The starting page in the global address space.
 * @param initData Initialize the contents to this data
 *
 * @return Device smart pointer of the created object.
 */
std::shared_ptr<Device> create24BitRomBankDevice(Byte startPage, const std::vector<Byte>& initData = {});

/**
 * @brief Create a 32-Bit ROM Bank object.
 *
 * @param startPage The starting page in the global address space.
 * @param initData Initialize the contents to this data
 *
 * @return Device smart pointer of the created object.
 */
std::shared_ptr<Device> create32BitRomBankDevice(Byte startPage, const std::vector<Byte>& initData = {});

/**
 * @brief Create a 16-Bit RAM Bank object.
 *
 * @param startPage The starting page in the global address space.
 * @param initData Initialize the contents to this data
 *
 * @return Device smart pointer of the created object.
 */
std::shared_ptr<Device> create16BitRamBankDevice(Byte startPage, const std::vector<Byte>& initData = {});

/**
 * @brief Create a 24-Bit RAM Bank object.
 *
 * @param startPage The starting page in the global address space.
 * @param initData Initialize the contents to this data
 *
 * @return Device smart pointer of the created object.
 */
std::shared_ptr<Device> create24BitRamBankDevice(Byte startPage, const std::vector<Byte>& initData = {});

/**
 * @brief Create a 32-Bit ROM Bank object.
 *
 * @param startPage The starting page in the global address space.
 * @param initData Initialize the contents to this data
 *
 * @return Device smart pointer of the created object.
 */
std::shared_ptr<Device> create32BitRamBankDevice(Byte startPage, const std::vector<Byte>& initData = {});

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

/**
 * @brief Create a SystemInterface object.
 *
 * @param startPage The starting page in the global address space.
 * @param powerOffCallback Function to handle powerOff() logic.
 * @param resetCallback Function to handle reste() logic.
 */
std::shared_ptr<SystemInterface> createSystemInterfaceDevice(Byte startPage);

}}

#endif
