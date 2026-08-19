# Raspberry Pi Pico 74HC IC Tester

A collection of Raspberry Pi Pico C programs for basic functional testing of ten Texas Instruments 74HC-series logic ICs.

## Supported ICs

1. SN74HC00N - Quad 2-input NAND gate
2. SN74HC02N - Quad 2-input NOR gate
3. SN74HC04N - Hex inverter
4. SN74HC08N - Quad 2-input AND gate
5. SN74HC14N - Hex Schmitt-trigger inverter
6. SN74HC32N - Quad 2-input OR gate
7. SN74HC138N - 3-to-8 decoder/demultiplexer
8. SN74HC164N - 8-bit serial-in, parallel-out shift register
9. SN74HC165N - 8-bit parallel-in, serial-out shift register
10. SN74HC595N - 8-bit serial-in, parallel-out shift register with storage register

> If a kit label says `SH74HC165N`, verify the actual chip marking. The TI part covered here is `SN74HC165N`.

## Electrical nuances

- Test one IC at a time.
- Power the DUT from the Pico's 3.3 V output.
- Connect the DUT ground to Pico ground.
- Add a 0.1 uF ceramic bypass capacitor between DUT VCC and GND, close to the IC.
- Do not leave CMOS inputs floating.
- Disconnect power before inserting, removing, or rewiring an IC.

These programs perform digital functional tests. They are not parametric production testers and do not measure propagation delay, current drive, noise margins, or analog thresholds. The SN74HC14 test verifies static inversion only; it does not characterize Schmitt-trigger hysteresis.

## Repository layout


├── CMakeLists.txt
├── README.md
├── docs/
│   └── WIRING.md
├── scripts/
│   ├── build_all.sh
│   └── clean.sh
├── src/
│   ├── sn74hc00_test.c
│   ├── sn74hc02_test.c
│   ├── sn74hc04_test.c
│   ├── sn74hc08_test.c
│   ├── sn74hc14_test.c
│   ├── sn74hc32_test.c
│   ├── sn74hc138_test.c
│   ├── sn74hc164_test.c
│   ├── sn74hc165_test.c
│   └── sn74hc595_test.c
└── uf2/

## Build all programs

Set the Pico SDK path first:

export PICO_SDK_PATH=~/pico/pico-sdk

Then:

./scripts/build_all.sh

The compiled `.elf` and `.uf2` files are generated in `build/`. The script also copies the UF2 files into `uf2/` so they can be committed to the repository for direct flashing.

## Build one tester

cmake -S . -B build
cmake --build build --target sn74hc00_test -j$(nproc)

Replace `sn74hc00_test` with the target you need.

## USB serial

Every tester uses Pico USB stdio and waits for a USB CDC serial monitor before running. On WSL, attach the running Pico USB device to WSL and open its `/dev/ttyACM*` device with a serial terminal such as `minicom`.

Example:

minicom -b 115200 /dev/ttyACM0

## Wiring

See [`docs/WIRING.md`](docs/WIRING.md).
