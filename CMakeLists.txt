cmake_minimum_required(VERSION 4.2)
project(mos6502_emulator VERSION 0.1 LANGUAGES CXX)

# Build settings
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ------------------------------
# External Deps
# ------------------------------

include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(nlohmann_json)

# ------------------------------
# Emulator library
# ------------------------------

add_library(mos6502_lib
    src/processor.cpp
    src/bus.cpp
    src/system.cpp
    src/frame.cpp
    src/devices/device_factories.cpp
    src/devices/device.cpp
    src/devices/memory.cpp
    src/devices/memory_bank.cpp
    src/devices/system_interface.cpp
    src/devices/display.cpp
    src/devices/ascii_display.cpp
    src/devices/pixel_display.cpp
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
    -fsanitize=undefined,address
)

# ------------------------------
# Main Executable
# ------------------------------

add_executable(mos6502
    src/main.cpp
)

target_link_libraries(mos6502 PRIVATE
    mos6502_lib
    nlohmann_json::nlohmann_json
)

target_link_options(mos6502 PRIVATE
    -fsanitize=undefined,address
)

# ------------------------------
# Tests
# ------------------------------

enable_testing()
add_subdirectory(tests)
