# Building, testing and flashing

## Prerequisites

| Tool | Why | Notes |
|---|---|---|
| `arm-none-eabi-gcc` toolchain | Cross-compiles the firmware | On Debian/Ubuntu: `sudo apt install gcc-arm-none-eabi`. Verified with GCC 13.2.1. |
| GNU `make`, `git`, Python 3 | Build system, submodules, libopencm3 header generation | |
| Host `gcc`/`g++` | Builds the unit tests | |
| `pre-commit` (optional) | Runs `clang-format` before you commit | `pip install pre-commit && pre-commit install` |

Windows users can build in Docker with the community
[ZombieBuild](https://github.com/crasbe/ZombieBuild) image. The upstream project also
ships a Code::Blocks project file (`stm32-vcu.cbp`) and a Gitpod configuration.

## Build

```bash
git clone --recurse-submodules https://github.com/damienmaguire/Stm32-vcu.git
cd Stm32-vcu
make get-deps      # builds libopencm3 for STM32F1 (only needed once)
make               # builds stm32_vcu, stm32_vcu.bin and stm32_vcu.hex
```

`make get-deps` is skipped automatically once `libopencm3/lib/libopencm3_stm32f1.a`
exists. If you cloned without `--recurse-submodules`, it runs
`git submodule update --init` for you.

A successful build ends with a size report like this (V2.41A):

```text
   text    data     bss     dec     hex filename
  75464    9828    3512   88804   15ae4 stm32_vcu
```

Useful variables:

| Command | Effect |
|---|---|
| `make V=1` | Show full compiler command lines |
| `make clean` | Remove `obj/` and the binaries |
| `make PREFIX=/opt/arm/bin/arm-none-eabi` | Use a toolchain that is not on `PATH` |

### Compiler flags worth knowing

From the `Makefile`:

- C++17, `-Os`, `-fno-rtti -fno-exceptions`: no `dynamic_cast`, no `try`/`throw`.
  Don't use the heap; every module is a static object.
- `-DSTM32F1 -mcpu=cortex-m3 -mthumb`.
- `-DMAX_USER_MESSAGES=30`: each CAN interface can register at most 30 receive IDs
  (see [CAN subsystem](can.md#receive-filters)).
- `-ffunction-sections -fdata-sections` with `--gc-sections`: unused functions cost
  nothing, so it is fine to leave helper code in a class.

### Memory budget

The linker script `stm32_vcu.ld` places the application at `0x08001000` (the first 4 KiB
hold the bootloader) and gives it **120 KiB of flash and 20 KiB of RAM**.

| | Used (V2.41A) | Limit | Headroom |
|---|---|---|---|
| Flash (text + data) | ~85 KiB | 120 KiB | ~35 KiB |
| RAM (data + bss) | ~13 KiB | 20 KiB | ~7 KiB, shared with the stack |

Every module object exists all the time, selected or not, so a new class costs its full
size even when unused. Keep large tables `const` so they stay in flash.

## Unit tests

The tests are host-side C++ and currently cover the throttle code.

```bash
make Test          # builds test/test_vcu with the host compiler
./test/test_vcu    # prints each test and "All tests passed"
```

The test Makefile compiles `src/throttle.cpp`, `libopeninv/src/params.cpp` and
`my_string.c` for the host, so anything you want to test must not depend on libopencm3
hardware calls. To add a test, follow the pattern in `test/test_throttle.cpp` and add
the file to `test/test_list.h` and `test/Makefile`.

## Continuous integration

`.github/workflows/CI-build.yml` runs on every pull request: install the ARM toolchain,
`make get-deps`, `make`, upload `stm32_vcu.bin`/`.hex` as artifacts, then build and run
the unit tests. `.github/workflows/run-pre-commit.yml` runs `clang-format` (v20) through
pre-commit and fails if any C/C++ file is not formatted. Run `pre-commit run --all-files`
locally before pushing.

## Flashing

The application is linked to start after a bootloader, so a blank board needs the
bootloader first. The upstream README names two:

- **Serial bootloader** (`tumanako-inverter-fw-bootloader` v5.2), used for updates over
  the UART, for example from the ESP8266 web interface or the `updater.py` script.
- **CAN bootloader** (`stm32-CANBootloader`), for updates over CAN.

Ways to load `stm32_vcu.bin`:

1. **Web interface:** on the OpenInverter ESP8266 Wi-Fi module, use the firmware
   update page and upload `stm32_vcu.bin`.
2. **SWD/JTAG:** with an ST-Link or similar, write `stm32_vcu.hex` (it carries its own
   addresses). The `make flash` target uses OpenOCD with a parallel-port adapter
   configuration you will almost certainly need to change.
3. **CAN:** with the CAN bootloader and a host tool that speaks its protocol.

!!! tip "Keep your parameters"
    Parameters live in their own flash page, separate from the program, and are matched
    by ID, so a firmware update normally keeps them. Still, save a JSON copy from the web
    interface before every update.

## Branches upstream

The upstream `CONTRIBUTING.md` defines three branches:

| Branch | Meaning |
|---|---|
| `master` | Validated on vehicles, release candidate. Don't open PRs here unless asked. |
| `Vehicle_Testing` | Under vehicle testing. Base most contributions here. |
| `Developmental_NOT_VEHICLE` | Compiles, otherwise untested. For larger frameworks. |

See [Contributing upstream](contributing.md).
