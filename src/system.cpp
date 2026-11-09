#include "system.hpp"
#include "bus.hpp"
#include "devices/device.hpp"
#include "processor.hpp"
#include <functional>
#include <thread>

namespace Emulator {

System::System(std::vector<std::shared_ptr<Device>> deviceList, std::shared_ptr<Display> display) :
    bus(),
    processor( std::bind(&Bus::read, &bus, std::placeholders::_1), std::bind(&Bus::write, &bus, std::placeholders::_1, std::placeholders::_2) ),
    display(display),
    deviceList(deviceList) {

    if (display) {
        display->bindIRQ(std::bind(&Mos6502::requestIRQ, &processor));
    }

    for (auto d : deviceList) {
        bus.registerDevice(d);
        if (d->type() == DeviceType::SYSTEM_INTERFACE) {
            d->bindReset(std::bind(&Mos6502::requestReset, &processor));
            d->bindPowerOff(std::bind(&System::powerOff, this));
        }
    }
}

bool System::powerStatus() { return powered; }

void System::powerOn() {
    powered = true;
    processor.powerOn();
}

void System::powerOff() { powered = false; }

void System::run(std::ostream& os) {
    if (!powered) return;

    using clock = std::chrono::steady_clock;

    const auto frame_interval = std::chrono::milliseconds(display ? display->getRefreshRateInMs() : 1000);
    auto next_frame = clock::now() + frame_interval;
    const uint64_t cycles_per_frame = 1000000 * display->getRefreshRateInMs() / 1000;
    uint64_t expected_cycles = cycles_per_frame;

    os << "\033[2J\033[1;1H";
    while (powered) {
        while (processor.getCumulativeCycles() < expected_cycles) {
            processor.step();
        }

        if (display)
            display->render().draw(os);

        if (clock::now() < next_frame) {
            std::this_thread::sleep_until(next_frame);
        }

        next_frame += frame_interval;
        expected_cycles += cycles_per_frame;
    }
    os << "Total runtime cycles: " << processor.getCumulativeCycles() << "\n";
}

}
