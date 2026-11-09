# MOS 6502 Emulator
An instruction-accurate emulator of the official 6502 spec.

## Features
- **Full Instruction Set:** Implements all official 6502 opcodes.
- **Addressing Modes:** Supports all addressing modes.
- **Accurate Flag Handling:** Precise behavior for all flags in the status register.
- **Configurable Address Space:** The 64KB address space can be mapped to custom device configurations.
- **Tested:** Instruction behavior is verified by a rigorous set of test cases.

## Architecture and Design
This emulator is designed around a modular, decoupled architecture inspired by concepts seen in real hardware.
The system is split into four primary components: the processor, the bus, virtualized hardware devices, and the system class.

### The Processor
- **Cycle count accuracy:** The processor tracks cumulative execution cycles, accounting for the number of cycles each instruction requires to execute (including page cross overhead). The cycle count is used for pacing execution speed.
- **Decoupled I/O via Callbacks:** The processor interacts with the address space using callback methods provided to it. This keeps the design modular and decoupled as the processor needs no external knowledge of the bus or how the address space is configured.

### The Bus
- **Page-Mapped Address Space:** The 16-bit address space is partitioned into 256 independent pages, where each page is 256 bytes.
- **Dynamic Device Registration:** The bus maintains an array of 256 page pointers. Devices register themselves by claiming a span of page slots in the array.
- **Open-Bus Behavior:** The bus checks bounds and resolves reads/writes by dispatching the operations to the appropriate device. If an address is not mapped to a device, read operations safely return a static sentinel value (0xFF) and writes are ignored.

### Virtualized Hardware Devices
- **Polymorphic Design:** All system components (RAM, ROM, memory banks, displays, and the system interface) inherit from an abstract Device base class.

### The System Class
- **Event Binding & Wiring:** The System class ties all the other components together at startup. It registers all devices defined in the config file onto the bus and binds device interrupt hooks to the processor.
- **Clock & Frame Synchronization:** The System class paces execution against real time by computing expected cycles per an interval and using a sleep method to maintain a steady emulation speed.

## Getting Started
### Requirements
- A C++20 compliant compiler
- CMake

### Build and Run
Project build
```
mkdir build && cd build
cmake ..
cmake --build .
```

Run a program
```
./mos6502 -c path/to/config.json
```
A sample program can be found under roms/conway_game_of_life/

If you'd like to run the tests, the binary is located at build/tests/unit_tests

## Compatibility & Design Notes
- **Indirect Jump Behavior:** The original 6502 processor had a bug with the JMP instruction where an indirect jump vector crossing a page boundary (such as 0x02FF->0x0300) wraps around within the same page instead of carrying over to the next page. As this is unintended behavior, this emulator does not feature this hardware bug.
