#include "frame.hpp"
#include <iostream>

namespace Emulator {

std::ostream& operator<<(std::ostream& os, Color color) {
    switch (color) {
        case Color::white:         return os << "\033[37m";
        case Color::red:           return os << "\033[31m";
        case Color::green:         return os << "\033[32m";
        case Color::blue:          return os << "\033[34m";
        case Color::yellow:        return os << "\033[33m";
        case Color::magenta:       return os << "\033[35m";
        case Color::cyan:          return os << "\033[36m";
        case Color::black:         return os << "\033[30m";

        case Color::bright_white:  return os << "\033[97m";
        case Color::bright_red:    return os << "\033[91m";
        case Color::bright_green:  return os << "\033[92m";
        case Color::bright_blue:   return os << "\033[94m";
        case Color::bright_yellow: return os << "\033[93m";
        case Color::bright_magenta:return os << "\033[95m";
        case Color::bright_cyan:   return os << "\033[96m";
        case Color::bright_black:  return os << "\033[90m";

        case Color::reset:         return os << "\033[0m";
    }

    return os;
}

void Frame::draw(std::ostream& os) const {
    size_t i = 0;
    size_t lines = 0;
    os << "\033[H";
    for (auto p : data) {
        os << p.color << p.glyph;
        ++i;
        if (i % width == 0) {
            os << '\n';
            ++lines;
        }
        if (lines >= height)
            break;
    }
    os << '\n' << Color::reset;
}

}
