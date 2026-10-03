# ZombieVerter (Stm32-vcu) documentation

The **ZombieVerter** is an open-source vehicle control unit (VCU) for electric vehicle
conversions. It runs the [Stm32-vcu](https://github.com/damienmaguire/Stm32-vcu) firmware
by Damien Maguire and contributors, built on Johannes Huebner's OpenInverter libraries.
It sits between the driver, the donor drive unit, the battery and the original car, and
makes them work as one vehicle.

This site documents that firmware in two halves:

<div class="grid cards" markdown>

-   :material-car-electric: **[User Guide](user/index.md)**

    For people installing and configuring a ZombieVerter: wiring, first power-up,
    every setting, charging, and troubleshooting.

-   :material-code-braces: **[Developer Guide](dev/index.md)**

    For people changing the firmware: how to build it, how the code is organised,
    how scheduling and CAN work, and step-by-step guides to adding a vehicle,
    inverter or BMS.

-   :material-table: **[Reference](reference/index.md)**

    Every parameter and spot value with its range, default and meaning, plus
    CAN IDs by module and the connector pinout.

</div>

## Which firmware this describes

| | |
|---|---|
| Firmware | Stm32-vcu **V2.41A** |
| Source commit | [`b061f84`](https://github.com/damienmaguire/Stm32-vcu/commit/b061f840ea05aa3e6c761953e746dabed9d9a375) on `master` (September 2026) |
| Libraries | libopeninv `3a9c059`, libopencm3 `3413ef8` (git submodules) |
| Hardware | ZombieVerter V1 / V1.3 board (STM32F105/107 class MCU) |

Everything here was written by reading that source code. Statements about behaviour come
from the code, not from testing on a vehicle. Where the code looks surprising, the
[Known issues](dev/known-issues.md) page says so and explains why.

!!! warning "Safety"
    The ZombieVerter switches high-voltage contactors and commands drive torque. A wrong
    setting can drive a car into a wall or weld a contactor. Test with the drive wheels off
    the ground, on low voltage first, and keep an independent way to open the HV circuit.
    This documentation is not a substitute for understanding your own system.

## Other sources

- Firmware repository and releases: <https://github.com/damienmaguire/Stm32-vcu>
- Hardware files (schematics, Gerbers, pinout) are in the firmware repository's
  `Hardware/` folder.
- The OpenInverter forum and wiki are the community's main discussion venue and are
  linked from the firmware README.
