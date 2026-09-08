#ifndef FRAME_HPP
#define FRAME_HPP

#include <cstddef>
#include <span>
#include <iostream>

namespace Emulator {

enum class Color {
    black,
    red,
    green,
    yellow,
    blue,
    magenta,
    cyan,
    white,

    bright_black,
    bright_red,
    bright_green,
    bright_yellow,
    bright_blue,
    bright_magenta,
    bright_cyan,
    bright_white,

    reset
};

struct Pixel {
    Color color;
    char glyph;
};

struct Frame {
    size_t width;
    size_t height;
    std::span<const Pixel> data;
    void draw(std::ostream& os) const;
};

}

#endif
