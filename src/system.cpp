#include "system.hpp"
#include "devices/device.hpp"
#include "processor.hpp"
#include <functional>
#include <thread>
#include <iomanip>

namespace Emulator {

System::System(std::vector<std::shared_ptr<Device>> deviceList, std::shared_ptr<Display> display) :
    bus(std::make_shared<Bus>()),
    processor(std::make_shared<Mos6502>(bus)),
    display(display),
    deviceList(deviceList) {

    if (display) {
        display->bindIRQ(std::bind(&Mos6502::requestIRQ, processor.get()));
    }

    for (auto d : deviceList) {
        bus->registerDevice(d);
        if (d->type() == DeviceType::SYSTEM_INTERFACE) {
            d->bindReset(std::bind(&Mos6502::requestReset, processor.get()));
            d->bindPowerOff(std::bind(&System::powerOff, this));
        }
    }
}

bool System::powerStatus() { return powered; }

void System::powerOn() {
    powered = true;
    processor->powerOn();
}

void System::powerOff() { powered = false; }

void System::run(std::ostream& os) {
    if (!powered) return;

    using clock = std::chrono::steady_clock;

    const auto frame_interval = std::chrono::milliseconds(display->getRefreshRateInMs());
    auto next_frame = clock::now() + frame_interval;
    const uint64_t cycles_per_frame = 1000000 * display->getRefreshRateInMs() / 1000;
    uint64_t expected_cycles = cycles_per_frame;

    os << "\033[2J\033[1;1H";
    while (powered) {
        while (processor->getCumulativeCycles() < expected_cycles) {
            processor->step();
        }
        processor->requestIRQ();
        display->render().draw(os);

        if (clock::now() < next_frame) {
            std::this_thread::sleep_until(next_frame);
        }

        next_frame += frame_interval;
        expected_cycles += cycles_per_frame;
    }

    for (int row = 0; row < 16; ++row) {
        for (int col = 0; col < 16; ++col) {
            os  << std::hex
                << std::setw(2)
                << std::setfill('0')
                << static_cast<unsigned>(bus->read(0x4000 + (row*16 + col)))
                << ' ';
        }
        os << '\n';
    }
}

}
