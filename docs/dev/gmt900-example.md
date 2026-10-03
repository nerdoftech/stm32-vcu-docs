# Worked example: GM GMT900

This page applies [Adding a vehicle class](adding-a-vehicle.md) to a GM GMT900 truck
(2007–2014 Silverado, Sierra, Tahoe and relatives) converted to a Tesla drive unit with
an OpenInverter controller. The engine computer (ECM) and the 6L80 gearbox computer
(TCM) are removed, so the VCU must recreate the frames they used to send on the
high-speed GMLAN bus.

!!! danger "Unverified frame contents"
    The IDs, rates and byte meanings below come from captures of **other** GM vehicles
    shared in public forums. None has been checked on a GMT900. The example class ships
    with only four frames enabled, and the undecoded frames send nothing until you paste
    in bytes captured from a stock truck. Capture first, then enable one frame at a time.

The complete, compile-tested source is in this repository under
[`examples/gmt900/`](https://github.com/nerdoftech/stm32-vcu-docs/tree/main/examples/gmt900):
`GM_GMT900.h`, `GM_GMT900.cpp`, and `register-vehicle.patch` (the edits to `Makefile`,
`param_prj.h` and `stm32_vcu.cpp`). They apply to a clean V2.41A tree, build with no
warnings, and pass the project's clang-format check.

```bash
cd Stm32-vcu
git apply /path/to/stm32-vcu-docs/examples/gmt900/register-vehicle.patch
cp /path/to/stm32-vcu-docs/examples/gmt900/GM_GMT900.h include/
cp /path/to/stm32-vcu-docs/examples/gmt900/GM_GMT900.cpp src/
make
```

## Bus plan

The GMT900 has two GMLAN buses: high-speed (500 kbit/s, dual wire, OBD pins 6/14: ECM,
TCM, EBCM, BCM) and low-speed (33.3 kbit/s single wire, OBD pin 1: cluster, radio,
HVAC). The BCM gateways between them. If, as on related GM platforms, the cluster's data
comes from high-speed frames, the VCU only needs to talk on the high-speed bus.

The VCU's modules can only be placed on CAN1 or CAN2 (see [CAN subsystem](can.md)), and
both run at 500 kbit/s, which suits high-speed GMLAN. A layout that keeps the truck's
traffic away from the drive unit:

| Bus | Carries | Parameters |
|---|---|---|
| CAN1 | OpenInverter board (0x3F out, 0x190 in, SDO), BMS, shunt, OBD2 responder, VCU SDO | `InverterCan=0`, `BMSCan=0`, `ShuntCan=0`, `OBD2Can=0`, `CanMapCan=0` |
| CAN2 | Truck high-speed GMLAN, vehicle class only | `VehicleCan=1` |

Reasons:

- Every selected module sees every frame from both buses (`CanCallback()` does not pass
  the bus). Keeping the truck on its own bus avoids an ID used by the truck being
  decoded by the inverter, BMS or charger driver. An ID clash of exactly this kind once
  made an OpenInverter car accelerate on its own.
- Leave the OBD2 responder off GMLAN so the VCU never answers a scan tool on the truck's
  behalf.
- CAN2 has a 120 Ω terminator fitted by default. GMLAN is already terminated at both
  ends, so if the VCU taps into the middle of the bus, remove the VCU's terminator and
  check the bus measures about 60 Ω with everything unplugged and powered off.
- Low-speed GMLAN would need CAN3 (MCP25625, which offers 33.3k), and in V2.41A CAN3 only
  forwards IDs 0x108/0x109. Avoid it unless a capture shows you must.

## Frames to recreate

| ID | Original sender | Contents (other GM vehicles) | Rate | Example class |
|---|---|---|---|---|
| 0x0C9 | ECM | RPM (bytes 2–3, 0.25 rpm/bit), run status, brake bit | ~80 Hz | **enabled**, run bit and brake bit position unverified |
| 0x0F9 | ECM/TCM | undecoded | 80 Hz | disabled |
| 0x1A1 | ECM | accelerator position | 40–80 Hz | disabled |
| 0x1C3 | ECM | torque / accelerator | 40 Hz | disabled; the EBCM likely wants it for StabiliTrak |
| 0x1ED | ECM | undecoded | 80 Hz | disabled |
| 0x1EF | ECM | RPM (alternate) | 80 Hz | disabled |
| 0x1F5 | TCM | PRNDL in byte 4 (1 P, 2 R, 3 N, 4 D), tow/haul in byte 6 | 40 Hz | **enabled** |
| 0x2C3 | ECM | undecoded | 20 Hz | disabled |
| 0x3C1, 0x3D1 | ECM | undecoded status | 10 Hz | disabled |
| 0x3F9, 0x3FB | ECM/TCM | undecoded | 4 Hz | disabled |
| 0x4C1 | ECM | coolant, intake air, outside air, `A − 40` °C | ~2 Hz | **enabled** |
| 0x4D1 | ECM | oil temp, possibly fuel level | ~2 Hz | **enabled**, fuel byte unknown |

Forum notes count bytes from 1; the code uses 0-based indexes (`bytes[3]` is "byte 4").

## Timing: 80 Hz frames

The scheduler offers 1, 10, 100 and 200 ms tasks and [cannot take a fifth](architecture.md#the-scheduler).
12.5 ms (80 Hz) is not a multiple of 10 ms, so the class runs its own schedule inside
`Task1Ms()`:

- Each frame has a period and a phase in **0.5 ms units** (12.5 ms = 25).
- A counter advances by 2 every millisecond. When it passes a frame's due time, the frame
  is built and sent and its due time moves on by one period. 12.5 ms frames therefore go
  out at alternating 12 and 13 ms intervals, averaging exactly 12.5 ms.
- Phases are staggered by 1.5 ms so no more than one frame is due on most ticks. The
  bxCAN has three transmit mailboxes and a 20-entry software queue; bursts larger than
  that are dropped.
- Total load with every frame enabled is about 490 frames/s, roughly 12 % of a 500 kbit/s
  bus.

```cpp
--8<-- "examples/gmt900/GM_GMT900.cpp:42:63"
```

## Mapping live values

| Truck function | VCU source | In the example |
|---|---|---|
| Tachometer (0x0C9) | `SetRevCounter()` (motor rpm, every 10 ms in Run/Charge) | Motor rpm ÷ `TACH_DIVIDER`, never below `TACH_IDLE_RPM` in Run so the BCM and HVAC see "engine running". |
| Gear display, reverse lamps, Park logic (0x1F5) | `Param::dir`, `opmode` | Park whenever not in Run; otherwise D/R/N from `dir`. |
| Coolant gauge (0x4C1) | `SetTemperatureGauge()` (inverter heatsink °C) | Held at 90 °C until the inverter is hotter than that. |
| Fuel gauge | `SetFuelGauge()` | Stored, not sent: the byte is unknown. Call `SetFuelGauge(Param::GetFloat(Param::SOC))` from a task once you find it. |
| Speedometer | `Veh_Speed` or wheel speeds | `DecodeCAN()` stores 0x3E9 as a placeholder. If the cluster takes speed from a powertrain frame, compute it from motor rpm and the axle ratio. |

The class sends nothing while the T15 ignition input is off, so the BCM can put the bus
to sleep at key-off.

## Gear selection and Park

The 6L80's internal mode switch went with the gearbox, so the column shifter's position
has to reach the VCU another way:

- **Forward/reverse inputs** (`dirmode`): simplest. The VCU then only knows D, R and N, so
  0x1F5 never shows P while driving. The example sends Park whenever the VCU is not in
  Run, which covers key-on before start.
- **`GetGear()`** in the vehicle class: read a Park switch (and others) from spare inputs
  and return all four positions. This takes priority over the forward/reverse inputs.

## Drive unit mounted reversed

If the drive unit is installed turned 180°, reverse rotation **on the OpenInverter
board**, not on the VCU. The VCU's `reversemotor` is forced back to 0 for every inverter
except `RearOutlander`, and flipping the VCU's `dirmode` would also flip `dir`, so the
cluster would show R while driving forward and the reverse lamps would come on.

## OpenInverter-specific points

- **The OI board does the torque math.** With `Inverter = OpenI` the VCU forwards raw
  pedal values in 0x3F; the inverter's own throttle, regen and current limits apply.
  Set and test those on the OI board. See [Throttle pipeline](throttle.md#openinverter-is-different).
- **0x3F is the hardened control message**, with two rolling counters and a CRC. Leave
  the inverter's counter and CRC checks enabled.
- **Precharge needs `udc`.** The VCU does not power the OI board (`inv_out`) during
  precharge and never copies the OI board's voltage into `udc`. Use a shunt (ISA or a
  contactor box) as the precharge reference, or power the OI board from ignition and add
  a CAN map from frame 0x190 (`udc` is in bits 8–23 at ×10 when `ConfigCANOI` set it up:
  `can rx udc 0x190 8 16 0.1`). The shunt is the more robust choice. *(Inferred from the
  code; test it on the bench.)*
- **`ConfigCANOI` wipes the OI board's CAN map** before writing its own 0x190 frame.
  Do it before any custom mapping on the inverter.

## Bring-up order

1. Log the stock truck on high-speed GMLAN: key-on, idle, every gear, a short drive.
2. Unplug the ECM, then the TCM, and note which IDs disappear. That is the spoof list
   for this truck.
3. Paste captured idle payloads into `Build()` for the undecoded frames, keep them
   disabled.
4. With HV disconnected, enable the four known frames and check DTCs and the cluster.
5. Enable the others one at a time, watching which warnings clear. If a module rejects a
   replayed frame, look for a rolling counter or checksum (`rollingCtr` is there for that).
6. Only then replace fixed bytes with live values.

Public references worth checking: the GM Global A DBC files in comma.ai's `opendbc`
repository, and GM standard GMW8762 for common 11-bit IDs.
