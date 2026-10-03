# General purpose I/O

Many pins can be given a job with a parameter. Change the parameter and the pin takes the
new function immediately; `save` to keep it. Reboot after moving a function from one
pin to another, because the old pin keeps the function until restart in V2.41A.

## Outputs

| Parameter | Pin | Type | Default |
|---|---|---|---|
| `Out1Func` | 31 GP Out 1 | low-side switch | `CoolantPump` (6) |
| `Out2Func` | 4 GP Out 2 | low-side switch | `NegContactor` (7) |
| `Out3Func` | 3 GP Out 3 | low-side switch | `HeaterEnable` (3) |
| `SL1Func` | 39 Trans SL1 | low-side switch | `NoneOut` |
| `SL2Func` | 38 Trans SL2 | low-side switch | `NoneOut` |
| `PWM1Func` | 7 PWM 1 | 12 V push-pull | `NoneOut` |
| `PWM2Func` | 6 PWM 2 | 12 V push-pull | `RunIndication` (4) |
| `PWM3Func` | 5 PWM 3 | 12 V push-pull | `OBCEnable` (2) |

### Output functions

| Value | Function | On when |
|---|---|---|
| 0 | `NoneOut` | never |
| 1 | `ChaDeMoAlw` | CHAdeMO charge permission (CHAdeMO driver) |
| 2 | `OBCEnable` | External charger enable (`chargemodes = EXT_DIGI`) |
| 3 | `HeaterEnable` | Heater allowed: Run with `Control` ≥ 1, or Preheat. Also the heater's coolant pump. |
| 4 | `RunIndication` | `opmode` = Run |
| 5 | `WarnIndication` | Reserved; not driven in V2.41A |
| 6 | `CoolantPump` | Precharge, Run, Charge, Preheat (off in Off) |
| 7 | `NegContactor` | Precharge, Run, Charge, Preheat |
| 8 | `BrakeLight` | `potnom` below `RegenBrakeLight` (strong regen) |
| 9 | `ReverseLight` | `dir` is reverse |
| 10 | `BrakeVacPump` | Brake vacuum low (needs an analogue input set to `BrakeVacSensor`), Run only |
| 11 | `CoolingFan` | Inverter or charger temperature above `FanTemp`; off 5 °C below. Run and Charge only. |
| 12 | `HvActive` | Run, Charge or Preheat |
| 13 | `ShiftLockNO` | Shift lock may release (speed below `DirChangeRpm`, plus brake in `SpeedBrake` mode), Run only |
| 14 | `PreHeatOut` | Preheat running |
| 15 | `PwmTim3` | PWM pins only: raw PWM from `Tim3_Presc`, `Tim3_Period`, `Tim3_x_OC` |
| 16 | `CpSpoof` | PWM pins only: fake control pilot for LIM/FOCCCI/CPC |
| 17 | `GS450pump` | PWM pins only: Toyota gearbox oil pump |
| 18 | `PwmTempGauge` | PWM pins only: temperature gauge, duty from `DC_MinTemp` to `DC_MaxTemp` |
| 19 | `PwmSocGauge` | PWM pins only: fuel gauge from SOC, duty from `DC_MinSOC` to `DC_MaxSOC` |

!!! note "PWM pins share one timer"
    If any PWM pin uses `CpSpoof`, `GS450pump`, `PwmTempGauge` or `PwmSocGauge`, all three
    PWM pins run at a fixed 1 kHz and the `Tim3_*` settings are overwritten.

## Digital inputs

| Parameter | Pin | Default |
|---|---|---|
| `GP12VInFunc` | 50 GP 12 V in | 12 (out of range; set it explicitly) |
| `HVReqFunc` | 51 HV request | `HVRequest` (2) |
| `PB1InFunc`, `PB2InFunc`, `PB3InFunc` | 42, 41, 40 Trans PB1–3 | `HVRequest` (2) |

| Value | Function | Effect |
|---|---|---|
| 0 | `NoneIn` | |
| 1 | `HeatReq` | Heater request switch (takes priority over a heater pot) |
| 2 | `HVRequest` | Bring up HV while on (external charge / service) |
| 3 | `DCFCRequest` | Reserved |
| 4 | `Switch_NoRegen` | Disable regen while on (for example a clutch switch) |
| 5 | `HVIL` | High-voltage interlock: must be on to leave Off; posts `HVILERR` when off |

!!! warning "With default settings, pin 51 is not the HV request input"
    Each input function can live on only one pin, and when several pins are given the
    same function the **last one assigned wins**. The order is GP 12 V, HV request, then
    PB1, PB2, PB3. Because PB1–PB3 also default to `HVRequest`, the HV request function
    ends up on **Trans PB3 (pin 40)**, an inverted input, and pin 51 does nothing for it.
    Set `PB1InFunc`, `PB2InFunc` and `PB3InFunc` to `NoneIn` (unless you use them) so
    that `HVReqFunc` on pin 51 takes effect. Also set `GP12VInFunc` to a valid value
    (0–5); its default of 12 is out of range.

The fixed inputs (start, brake, forward, reverse, ignition) are not configurable.

## Analogue inputs

| Parameter | Pin |
|---|---|
| `GPA1Func` | 9 Analogue 1 |
| `GPA2Func` | 8 Analogue 2 |

| Value | Function | Settings |
|---|---|---|
| 0 | `None` | |
| 1 | `ProxPilot` | Charge port proximity. `ppthresh`; reading shown in `PPVal`. |
| 2 | `BrakeVacSensor` | Brake vacuum. `BrakeVacThresh`, `BrakeVacHyst`; reading in `BrkVacVal`. |
| 3 | `HeaterPot` | Heater control knob. `HeatPotDir`, `HeatPotOn`, `HeatPotFull`; reading in `HtPotVal`. |

The Subaru vehicle class reads the analogue inputs directly for its gear and cruise
switches.

## Tacho and speedo pulse output

The Oil Pump PWM pin (30) can output a square wave proportional to motor speed for an
old-style tachometer or speedometer: set `PumpPWM` to `TachoOut` or `SpeedoOut` and
`TachoPPR` to the pulses per revolution your gauge expects. The `Classic` and `Subaru`
vehicles use it.
