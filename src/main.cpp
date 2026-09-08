#include "bus.hpp"
#include "constructors.hpp"
#include <iostream>

#include <chrono>
#include <thread>
using cclock = std::chrono::steady_clock;
using ms = std::chrono::milliseconds;

using namespace Emulator;

int main() {
    Bus bus;
    auto display = Constructors::createPixelDisplayDevice(0x00, 0x30, 0x10, Emulator::Display::RefreshRate::EIGHT);
    bus.registerDevice(display);

    auto frame_duration = ms(display->getRefreshRateInMs());
    auto next_frame = cclock::now();

    for (int i = 0; i < display->getRefreshRate()*5; ++i) {
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

    std::cout << "Done\n";
}
