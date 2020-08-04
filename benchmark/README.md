# Overview

We benchmark our code w/ and w/o countermeasures on two different Cortex-M4 targets:
- The STM32F303 which is the core that we also used in our attacks and is part of the ChipWhisperer-Lite (CW1173) 32-bit basic
board.
- The more popular STM32F407 which is used in various post-quantum cryptography implementations in the literature and
the benchmarking project [PQM4](https://github.com/mupq/pqm4).

Both cores implement implement the Cortex-M4 architecture, so they should have in theory have the same instruction timings.
However, they do have different memory and cache architectures which do behave slightly differently.

# STM32F303 (the CW1173 target)
You are required to have a [CW1173](https://wiki.newae.com/CW1173_ChipWhisperer-Lite) at hand to run the following.

## Installation
- Follow the instructions on https://wiki.newae.com/Installing_ChipWhisperer
- If you are lucky, `pip install chipwhisperer` and installing `arm-none-eabi-gcc` is sufficient.

## Running Benchmarks
- `cd cwlitearm`
- running `python3 ./benchmark.py` will do the following
  - build `speed_nocountermeasures-CWLITEARM.elf` (no countermeasures), `speed_countermeasures_notwistcm.elf` (countermeasures except for those protecting against the 'twist attack'), `speed_countermeasures_twistcm` (all countermeasures)
  - use the chipwhisperer Python library to flash the binaries to the board (make sure it's plugged in)
  - print the resulting output to the terminal

 - For details of what the benchmark is doing, see [speed.c](./cwlitearm/speed.c).

 If you do not have a CW1173 right now, you look at the benchmarking results at
 [cwlitearm/speed_nocountermeasures.log](./cwlitearm/speed_nocountermeasures.log),
 [cwlitearm/speed_countermeasures_notwistcm.log](./cwlitearm/speed_countermeasures_notwistcm.log), and
 [cwlitearm/speed_countermeasures_twistcm.log](./cwlitearm/speed_countermeasures_twistcm.log),

# STM32F407 (the pqm4 target)
You are required to have a [STM32F407 Disovery Board](https://www.st.com/en/evaluation-tools/stm32f4discovery.html) and a USB-TTL converter.

## Installation
- Follow the insturctions on https://github.com/mupq/pqm4
- If you are lucky, installing `arm-none-eabi-gcc` and [st-link](https://github.com/stlink-org/stlink) is sufficient.

## Running Benchmark
- `cd stm32f407`
- `make` will build the binaries `speed_nocountermeasures.bin` (no countermeasures), `speed_countermeasures_notwistcm.bin` (countermeasures except for those protecting against the 'twist attack'), and `speed_countermeasures_twistcm.bin` (all countermeasures)
- You can flash the binaries using
   - `st-flash write speed_nocountermeasures.bin 0x8000000`
   - `st-flash write speed_countermeasures_notwistcm.bin 0x8000000`
   - `st-flash write speed_countermeasures_twistcm.bin 0x8000000`
 - To receive the output, you can either use the provided Python script or `screen`
   - `./host_unidirectional.py`
   - `screen /dev/ttyUSB0 115200`
   - You may have to adjust the USB device accordingly.

 - For details of what the benchmarks are doing, see [speed.c](./stm32407/speed.c).

If you do not have a STM32F407 Disovery Board right now, you look at the benchmarking results at
[stm32f407/speed_nocountermeasures.log](./stm32f407/speed_nocountermeasures.log),
[stm32f407/speed_countermeasures_notwistcm.log](./stm32f407/speed_countermeasures_notwistcm.log), and
[stm32f407/speed_countermeasures_twistcm.log](./stm32f407/speed_countermeasures_twistcm.log).
