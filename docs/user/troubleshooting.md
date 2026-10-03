# Errors and troubleshooting

## Reading errors

- `lasterr` shows the most recent error.
- The terminal command `errors` prints the error log with timestamps (seconds since
  power-up).
- `status` and `TorqDerate` are bit fields that explain why the VCU is waiting or
  limiting.

In Run and Charge the active error list is cleared every 10 ms, so `lasterr` and the log
are the places to look.

## Error list

| Error | Meaning | What to check |
|---|---|---|
| `PRECHARGE` | `udc` did not reach `udcsw` within 5 s of starting precharge. | See [Precharge fails](#precharge-fails). |
| `OVERVOLTAGE` | `udc` went above `udclim`. The VCU went to Off. | `udclim` too low for a full pack; wrong shunt scaling; regen at full charge. |
| `THROTTLE1`, `THROTTLE2` | A pedal channel is more than 200 counts outside its calibrated range (only posted in Run). | Wiring, 5 V supply (pin 48), `potmin`/`potmax`. |
| `THROTTLE12` | Both channels out of range. Throttle is zero. | As above. |
| `THROTTLE12DIFF` | Dual channels differ by more than 10 %. Limp mode (≤ 50 %). | `pot2min`/`pot2max` calibration. |
| `THROTTLEMODE` | Invalid `potmode`. | Set 0 or 1. |
| `CANTIMEOUT` | `canio` was in use and nothing arrived on the inverter's bus for ~1 s. | Inverter CAN wiring, termination. |
| `TMPHSMAX`, `TMPMMAX` | Inverter or motor above its limit; torque derated. | Cooling, `tmphsmax`/`tmpmmax`. |
| `HVILERR` | The input set to `HVIL` is off. | Interlock loop. Blocks starting; does not stop a running car. |

`BMSCOMM` and `PWMCONFIGERROR` exist in the list but are not raised by V2.41A.

## The `status` bits

| Bit | Name | Meaning |
|---|---|---|
| 4 | `UdcBelowUdcSw` | `udc` < `udcsw` (precharge not complete) |
| 8 | `UdcLim` | `udc` ≥ `udclim` |
| 64 | `PotPressed` | `pot` > `potmin` (pedal not released) |

Precharge completes only when all three are clear.

## The car won't start (stays in Off)

Check, in this order:

1. `T15Stat` = 1 with the key on. If not: ignition wiring (pin 15), or the vehicle class
   expects ignition over CAN.
2. `din_start` goes to 1 when you turn the key to start. If not: pin 52 wiring.
3. `pot` is **below** `potmin` with your foot off. The default `potmin` is 0, which can
   never be satisfied; calibrate the throttle.
4. If an input is set to `HVIL`, it must be on.
5. `Vehicle` is set to your car (the default is BMW E46, which takes ignition from CAN).

## Precharge fails

`opmode` goes 2 then 3, `lasterr` = `PRECHARGE`.

1. Is anything measuring voltage? With `ShuntType = 0` and anything other than a Leaf
   inverter, `udc` stays 0. Fit a shunt or map `udc` from CAN.
2. Watch `udc` during precharge. Does it rise at all? If not: precharge relay, resistor,
   HV fuse, shunt wiring (U1 must be on the inverter side).
3. Does it rise but stop short of `udcsw`? Lower `udcsw` or use a smaller precharge
   resistor. It must get there within 5 s.
4. Key off to leave PchFail, then try again.

## The motor doesn't turn

1. `opmode` = 1 (Run)?
2. `dir` = 1 or −1? Neutral and park give zero torque.
3. `potnom` rises with the pedal? If not, check calibration and `TorqDerate`.
4. OpenInverter: the inverter must see the VCU's frame 0x3F (CAN bus, `InverterCan`),
   and its own parameters must be set for CAN control. Check the inverter's own
   `opmode` and errors.
5. `InvStat` shows what the inverter reports.

## Torque is weaker than expected

Look at `TorqDerate`:

| Bit | Reason | Parameter |
|---|---|---|
| 1 | Pack voltage near `udcmin` | `udcmin` |
| 2 | Pack voltage near `udclim` (regen) | `udclim` |
| 4 | Regen current limit | `idcmin` |
| 8 | Discharge current limit | `idcmax` |
| 16 | Temperature | `tmphsmax`, `tmpmmax` |

Also check `throtmax`, `revlim` and `throtramp`. With an OpenInverter drive unit the
inverter's own limits apply.

## Charging doesn't start

1. `chargemodes` and `ChargerCan` correct, charger powered?
2. `Chgctrl`: `Enable`, or `Timer` with the clock set.
3. `PlugDet` if a proximity input is configured.
4. The charge lockout: after a normal charge end, `Enable` mode won't restart until you
   have driven (entered Run). Power-cycling the VCU also clears it.
5. BMS: `BMS_ChargeLim` > 0 and BMS data arriving (`BMS_Vmin` etc. updating).
6. Not in Run: AC charging can't start in Run.

## Charging stops early

- `udc` reached `Voltspnt` with `idc` ≤ `IdcTerm`.
- A cell hit `BMS_VmaxLimit`, or temperature limits.
- BMS data stopped for longer than `BMS_Timeout`.
- Timer mode: `Chg_Dur` expired.

## Something else

- `cpuload` well above 50 % suggests a module is overloading the scheduler.
- A CAN bus should measure about 60 Ω between H and L with power off.
- Compare your parameters with the examples in the firmware's `Parameters/` folder.
- When asking for help, include your parameter JSON, `version`, a CAN log and what you
  expected (the upstream contributing guide asks for these).
