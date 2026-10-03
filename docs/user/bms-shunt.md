# BMS, shunt and state of charge

## Shunts and contactor boxes (`ShuntType`)

The shunt is the VCU's main source of battery voltage and current. Precharge, voltage
limits, current limits, charge termination and SOC all depend on it.

| `ShuntType` | Device | Provides |
|---|---|---|
| 0 `None` | | Nothing. See "Without a shunt" below. |
| 1 `ISA` | Isabellenhütte IVT-S | `udc` (U1), `udc2`, `udc3`, `idc`, `power`, `KWh`, `AMPh`, `tmpaux` |
| 2 `SBOX` | BMW i3 S-box | `udc` (output side), `udc2` (battery side), `idc`, `power`; also **switches the contactors over CAN** following the VCU's mode. Sets `udcsw` to battery voltage − 20 V. |
| 3 `VAG` | VW e-box | `udc`, `udc2`, `idc`; also switches contactors. Sets `udcsw` to battery voltage − 20 V. |
| 4 `ISA_udcsw` | ISA IVT-S | As ISA, and sets `udcsw` to U2 − 20 V whenever U2 is above `udcmin`. |

Wire the ISA so **U1 measures the inverter side** of the main contactor: that is the
voltage that rises during precharge. With `ISA_udcsw`, U2 should measure the battery
side.

A new ISA shunt needs a one-time setup: set `IsaInit = 1`, `save`, `reset`, then set
`IsaInit = 0` and `save` again.

### Without a shunt

With `ShuntType = 0`, `udc` only comes from:

- the Nissan Leaf inverter driver, or
- the Mitsubishi Outlander charger driver (when charging), or
- a CAN map you create (`can rx udc ...`).

Otherwise precharge can never complete. SimpBMS without a shunt does provide pack
voltage (`udc2`), current (`idc`) and an automatic `udcsw`, but not `udc` itself.

## State of charge

`SOC` = 100 % − 100 % × `KWh` used ÷ `BattCap`. It needs a shunt that counts energy (ISA)
and `BattCap` set to your usable pack energy. It resets only when the shunt's counter
resets. SOC drives fuel gauges in vehicle classes that support it and the PWM SOC gauge
output.

## BMS (`BMS_Mode`)

| `BMS_Mode` | BMS | CAN IDs |
|---|---|---|
| 0 `Off` | none | |
| 1 `SimpBMS` | SimpBMS and compatible | 0x351, 0x355, 0x356, 0x373 |
| 2 / 3 `TiDaisychainSingle` / `Dual` | Daisy-chain BMS (one or two units) | 0x4F1, 0x4F5 |
| 4 `LeafBms` | Nissan Leaf LBC | Leaf IDs |
| 5 `RenaultKangoo33` | Renault Kangoo 33 kWh | 0x155, 0x424, 0x425, 0x7BB |

What the VCU does with BMS data:

- Shows `BMS_Vmin`, `BMS_Vmax`, `BMS_Tmin`, `BMS_Tmax` and `BMS_ChargeLim`.
- **Stops AC charging** when data stops for `BMS_Timeout` seconds, when any limit
  (`BMS_VminLimit`, `BMS_VmaxLimit`, `BMS_TminLimit`, `BMS_TmaxLimit`) is exceeded, or when
  the BMS's own charge limit is 0.

What it does **not** do in V2.41A: reduce drive power or regen from BMS data. Protect the
pack while driving with `udcmin`, `idcmax`/`idcmin`, and the inverter's own limits.

## Tesla modules

There is no built-in driver for Tesla Model S/X module boards or Model 3 battery
electronics. A common approach is SimpBMS (or a compatible BMS) reading the modules and
reporting to the VCU with the SimpBMS protocol.
