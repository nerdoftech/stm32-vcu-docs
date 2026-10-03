# Code quirks and known issues

Things noticed while reading the V2.41A source that can surprise a user or a developer.
They are observations from the code, not confirmed bug reports; check the upstream issue
tracker and the `Vehicle_Testing` branch before fixing any of them, as some may already
be addressed there.

## Vehicle "None" is 3 in the UI but 4 in code

`VEHMODES` offers `3=None`, while `enum vehicles` has `None = 4`, and `UpdateVehicle()`
has no `case 3`. Selecting 3 leaves the previously selected vehicle active. After a
reboot with `Vehicle = 3` the boot default (`NoVehicle`) is used, so it behaves as None
in practice, but switching to 3 at runtime does not deselect a vehicle class.

Also note the **default `Vehicle` is 0 = BMW_E46**, so a fresh board runs the E46 dash
code until you change it.

## Options with no implementation

- `Inverter = 3 (UserCAN)` has no case in `UpdateInv()`.
- `Heater = 6 (PWM)` has no case in `UpdateHeater()`.
- `ChargeModes::MGgen2 = 7` exists in the enum but not in the option string or
  `UpdateCharger()`.

Selecting an unimplemented option keeps the previous module.

## `DirChange` condition is always true

In `utils::SelectDirection()`:

```cpp
} else if (ChangeLim == 1 || 2)
```

`|| 2` is always true. Because `ChangeLim == 0` is handled by the branch above, and modes
1 and 2 are told apart inside this branch, there is no visible effect today, but the
intent was clearly `ChangeLim == 1 || ChangeLim == 2`.

Also in that function, `prevValidDir` is a local initialised to 0 on every call, so the
"don't re-check when going back to the previous direction" logic never remembers
anything between calls.

## I/O matrix reset loops

In `IOMatrix::AssignFromParams()`:

```cpp
for (int i = 0; i < 8; i++)          functionToPinIn[i]  = &DigIo::dummypin;
for (int i = 8; i < numPins; i++)    functionToPinOut[i] = &DigIo::dummypin;
```

`functionToPinIn` has `LAST_IN` = 6 entries, so the first loop writes two entries past
its end (GCC warns: "iteration 6 invokes undefined behavior"). In a V2.41A build the
linker places `CRCByte`, `CCByte`, `canCtr100` and `SBOX::Voltage2` right after the
array, so those get overwritten every time a parameter changes. The exact victims depend
on the build.

The second loop resets output functions 8–12 only. Output functions 0–7 and 13–19 are
never reset to `dummypin`, so:

- when you move a function to another pin, the old pin keeps that function until reboot;
- an output function that no pin has ever been assigned to is a null pointer.
  `Ms100Task` calls `GetPinOut(SHIFTLOCKNO)->Clear()` and similar unconditionally, which
  dereferences that null pointer. Address 0 is readable on the STM32 (it aliases flash),
  so this does not crash, but it is undefined behaviour and writes to an unintended
  address.

If you add I/O functions, consider fixing both loops to run to `LAST_IN` / `LAST_OUT`.

## Default input functions collide

An input function maps to exactly one pin, and `AssignFromParams()` assigns in the order
`GP12VInFunc`, `HVReqFunc`, `PB1InFunc`, `PB2InFunc`, `PB3InFunc`, so the last pin
given a function wins. `HVReqFunc`, `PB1InFunc`, `PB2InFunc` and `PB3InFunc` all default
to `HVRequest` (2). With defaults, `GetPinIn(HVREQ)` is therefore `gear3_in` (Trans PB3,
pin 40, inverted), not `HV_req` (pin 51). The `din_HVreq` spot value reads `HV_req`
directly, so it can show 1 while the HV request logic ignores it.

## `GP12VInFunc` default above its max

`PARAM_ENTRY(CAT_IOPINS, GP12VInFunc, PININFUNCS, 0, 5, 12, 98)`: default 12, max 5,
and input functions only go up to 5. With the default, `AssignFromParams()` writes
`functionToPinIn[12]`, 24 bytes past the end of a 6-entry array. Set `GP12VInFunc`
explicitly on a new board.

## Precharge needs `udc` from somewhere

With `ShuntType = 0`, `udc` is only written by the Leaf inverter, the Outlander charger,
or a CAN map. With an OpenInverter drive unit and no shunt, precharge waits for a
voltage that never comes and fails after 5 s. See
[State machine](state-machine.md#finishing-precharge).

## `prechargeMinTime` is never reset

The 1 s minimum precharge time is a static that counts down once after boot. Later
precharges in the same power cycle have no minimum time.

## Pedal start interlock uses raw ADC

The Off → Precharge check is `pot < potmin` on the raw reading, so it assumes the reading
rises with pedal travel and requires `potmin` to be above the released-pedal value. See
[State machine](state-machine.md#leaving-off).

## BMS does not limit drive torque

All BMS drivers only gate charging. Nothing in the drive path reads the BMS's
discharge or regen limits.

## CAN3 is almost unused

The MCP25625 interrupt handler passes only IDs 0x108 and 0x109 to the charge interface
and drops everything else. It reads the frame before checking whether `CANSPI_receive()`
succeeded. The `...Can` selectors only allow CAN1 and CAN2.

## Same frame, both buses

`CanCallback()` does not know which bus a frame came from, so every selected module sees
frames from both buses. Two modules using the same ID on different buses will both react.

## Software send queue is LIFO

`Stm32Can::HandleTx()` drains the 20-entry overflow queue from the end, so frames that
overflowed the three hardware mailboxes go out in reverse order. Frames are dropped
silently when the queue is full.

## Saving is allowed in any mode

`TerminalCommands::saveEnabled` and `SdoCommands::saveEnabled` are never set false in
this project, so `save` works while driving. Writing flash disables interrupts and pauses
the scheduler.

## Unused or display-only parameters

`cruiselight`, `CCS_SOCLim` and the spot values `potbrake` and `brakepressure` are not
used by V2.41A code. `EnableTractionControl()` and `SetFuelGauge()` on the vehicle are
never called by the core.

## Compiler warnings

A clean build of V2.41A prints a few warnings worth knowing about:

- `iomatrix.cpp`: the out-of-bounds loop above.
- `throttle.cpp`: unused `SpeedFiltered`.
- `cansdo.cpp`: packed-struct pointer cast.
