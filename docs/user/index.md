# User Guide

This guide is for people fitting a ZombieVerter to a vehicle and setting it up. It
assumes you can wire 12 V automotive circuits and use a laptop, but not that you can
read C++.

## Suggested reading order

1. [What the ZombieVerter does](overview.md): the big picture and the list of supported
   parts.
2. [Hardware and wiring](hardware.md): connector, power, CAN buses, contactors.
3. [Firmware and tools](firmware.md): how to connect, update firmware, and save settings.
4. [First-time setup](first-setup.md): a step-by-step commissioning checklist from first
   power-up to first drive.
5. Then the topic pages as you need them: [operating modes](operating-modes.md),
   [throttle and regen](throttle.md), [charging](charging.md),
   [BMS and shunt](bms-shunt.md), [general purpose I/O](gpio.md),
   [heater and accessories](accessories.md), [CAN mapping](can-mapping.md).
6. [Errors and troubleshooting](troubleshooting.md) when something doesn't work.

Every setting is listed with its range and default in the
[parameter reference](../reference/parameters.md).

## Words used in this guide

**Parameter**
:   A setting you change, for example `potmax` or `Inverter`. Saved to flash with `save`.

**Spot value**
:   A live reading the VCU publishes, for example `udc` or `opmode`. Not saved.

**opmode**
:   The VCU's current state: Off, Precharge, Run, Charge, Preheat or PchFail.

**T15**
:   The ignition-on signal (from German terminal numbering: T15 = ignition, T30 =
    permanent battery, T50 = crank).

More in the [glossary](../reference/glossary.md).
