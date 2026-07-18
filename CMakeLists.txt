cmake_minimum_required(VERSION 4.2)
project(mos6502_emulator VERSION 0.1 LANGUAGES CXX)

# Build settings
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ------------------------------
# Emulator library
# ------------------------------

add_library(mos6502_lib
    src/bus.cpp
    src/ram.cpp
    src/rom.cpp
)

target_include_directories(mos6502_lib PUBLIC
    src/include
)

target_compile_options(mos6502_lib PRIVATE
    -Wall
    -Wextra
    -Werror
    -Wpedantic
    -O2
    -flto
)

# ------------------------------
# Main Executable
# ------------------------------

add_executable(mos6502
    src/main.cpp
)

target_link_libraries(mos6502 PRIVATE
    mos6502_lib
)

# ------------------------------
# Tests
# ------------------------------

enable_testing()
add_subdirectory(tests)
