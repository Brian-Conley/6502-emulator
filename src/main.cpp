#include "bus.hpp"
#include "util/constructors.hpp"
#include <iostream>

#include <chrono>
#include <thread>
using cclock = std::chrono::steady_clock;
using ms = std::chrono::milliseconds;

using namespace Emulator;

void testCallbackOne() {
    std::cout << "powerOff Callback\n";
}

void testCallbackTwo() {
    std::cout << "reset Callback\n";
}

int main() {
    Bus bus;
    auto display = Constructors::createPixelDisplayDevice(0x00, 0x30, 0x10, Emulator::Display::RefreshRate::EIGHT);
    bus.registerDevice(display);

    auto frame_duration = ms(display->getRefreshRateInMs());
    auto next_frame = cclock::now();

    std::cout << "\033[2J\033[H";
    for (int i = 0; i < display->getRefreshRate()*2; ++i) {
        std::stringstream ss;
        next_frame += frame_duration;
        if (i % 3 == 0) {
            for (int j = 0; j < 0x0300; ++j) {
                if ((j + 1) % 0x30 == 0 || j % 0x30 == 0 || j < 0x30 || j >= 0x2D0)
                    bus.write(j, 0b10011111);
                else
                    bus.write(j, 0b00000001);
            }
        } else if (i % 3 == 1) {
            for (int j = 0; j < 0x0300; ++j) {
                if ((j + 1) % 0x30 == 0 || j % 0x30 == 0 || j < 0x30 || j >= 0x2D0)
                    bus.write(j, 0b10101111);
                else
                    bus.write(j, 0b00000010);
            }
        } else {
            for (int j = 0; j < 0x0300; ++j) {
                if ((j + 1) % 0x30 == 0 || j % 0x30 == 0 || j < 0x30 || j >= 0x2D0)
                    bus.write(j, 0b10111111);
                else
                    bus.write(j, 0b00000011);
            }
        }
        auto f = display->render();
        f.draw(ss);
        std::cout << ss.str();
        std::this_thread::sleep_until(next_frame);
    }

    std::shared_ptr<SystemInterface> inter = std::make_shared<SystemInterface>(0xAB);
    inter->bindPowerOff(testCallbackOne);
    inter->bindReset(testCallbackTwo);
    inter->powerOn();
    bus.registerDevice(inter);
    // Reset
    bus.write(0xAB01, 0x01);
    // Power off
    bus.write(0xAB00, 0x01);

    std::cout << "Done\n";
}
