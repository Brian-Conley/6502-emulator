#ifndef SYSTEM_HPP
#define SYSTEM_HPP

#include "devices/device.hpp"
#include "devices/display.hpp"
#include "bus.hpp"
#include "processor.hpp"
#include <atomic>
#include <memory>
#include <vector>
#include <iostream>

namespace Emulator {

class System {
    public:
        System(std::vector<std::shared_ptr<Device>> deviceList, std::shared_ptr<Display> display = nullptr);

        bool powerStatus();
        void powerOn();
        void powerOff();

        void run(std::ostream& os);

    private:
        System() = delete;

        std::atomic<bool> powered;

        Bus bus;
        Mos6502 processor;
        std::shared_ptr<Display> display;
        std::vector<std::shared_ptr<Device>> deviceList;
};

}

#endif
