# Module interfaces

Each kind of plug-in module has an abstract base class in `include/`. The core in
`stm32_vcu.cpp` only ever calls these methods through the `selectedX` pointers. This
page lists every method, what it must do, and exactly when the core calls it.

Common to all of them:

- `SetCanInterface(CanHardware *c)` stores `c` in the protected `can` member. Override it
  to register receive IDs (call `can = c;` first). It is called whenever CAN filters are
  rebuilt: at boot, after any module change, and after any `...Can` bus change. Several
  existing classes also use it as an "init" hook.
- `DecodeCAN(int id, ...)` is called from the CAN receive interrupt for **every accepted
  frame on both buses**. Check the ID.
- `TaskXMs()` methods are called from the scheduler interrupt. Default implementations do
  nothing, so override only what you need.

## Vehicle (`vehicle.h`)

The car being converted: ignition and start signals, dash gauges, gear selector,
cruise buttons, and any CAN messages the original ECUs expect.

| Method | Required | Called from | Contract |
|---|---|---|---|
| `bool Ready()` | **yes** | `Ms100Task` (→ `T15Stat`); state machine every 10 ms | Ignition is on. Off → Precharge needs it; Run → Off when it goes false. |
| `bool Start()` | no (default `din_start`) | State machine in Off | Driver asked to start. BMW E65 returns its CAS state. |
| `void SetRevCounter(int speed)` | **yes** | `Ms10Task`, Run and Charge only | Absolute motor rpm. Drive the tach (CAN or pulse). |
| `void SetTemperatureGauge(float temp)` | **yes** | `Ms10Task`, always | Inverter temperature (`tmphs`) in °C. |
| `void SetFuelGauge(float level)` | no | Not called by the core | SOC 0–100 %. Classes call it themselves (E65 does in `Task200Ms`). |
| `bool GetGear(gear &g)` | no (returns false) | `SelectDirection()` every 100 ms | Return true and set `PARK/REVERSE/NEUTRAL/DRIVE` if the car's own selector is read by this class. Takes priority over the shifter class and the fwd/rev inputs. |
| `int GetCruiseState()` | no (`CC_NONE`) | `Ms100Task` | Bit field of `CC_ON`, `CC_CANCEL`, `CC_SET`, `CC_RESUME`. |
| `float GetFrontRearBalance()` | no (50) | `Ms100Task` → `FrontRearBal` | 100 = all front. |
| `bool EnableTractionControl()` | no (false) | Not called by the core | |
| `void DashOff()` | no | Every 10 ms while Off | Stop or reset dash output. |
| `void Task1Ms/10Ms/100Ms/200Ms()` | no | Every task, all modes | Periodic sends, timeouts. |
| `void DecodeCAN(int, uint32_t*)` | no | CAN ISR | Read the car's frames. |

Selected by the `Vehicle` parameter (`vehicles` enum). CAN bus: `VehicleCan`.

## Inverter (`inverter.h`)

The drive inverter.

| Method | Required | Called from | Contract |
|---|---|---|---|
| `void SetTorque(float torquePercent)` | **yes** | `Ms10Task`, every cycle in every mode (0 outside Run) | −100…100, already direction-signed. Send the command. Update `Param::torque` if useful. |
| `float GetMotorSpeed()` | **yes** | `Ms10Task` → `speed` | Signed rpm. |
| `float GetMotorTemperature()` | **yes** | `Ms100Task` → `tmpm` | °C. |
| `float GetInverterTemperature()` | **yes** | `Ms100Task` → `tmphs` | °C. Drives derate, fan and temp gauge. |
| `float GetInverterVoltage()` | **yes** | `Ms100Task` → `INVudc` | Volts. Display only; set `Param::udc` yourself if the inverter is your precharge reference (Leaf does). |
| `int GetInverterState()` | **yes** | `Ms100Task` → `InvStat` | Driver-defined. |
| `void DeInit()` | no | Before switching away | Undo pin/timer setup. |
| `void Task1Ms/10Ms/100Ms()` | no | `Task10Ms` only in Run; the others always | |
| `void DecodeCAN(int, uint32_t*)` | no | CAN ISR | |

Selected by `Inverter` (`InvModes`). CAN bus: `InverterCan`. Inverter power (`inv_out`)
is switched by the state machine, except that `OpenI` is not powered during precharge.

## BMS (`bms.h`)

The base class itself is the "no BMS" implementation.

| Method | Default | Called from | Contract |
|---|---|---|---|
| `float MaxChargeCurrent()` | 9999 | `Ms200Task` in Charge | 0 ends AC charging and latches the charge lockout. |
| `void Task100Ms()` | writes 0s to `BMS_*` | `Ms100Task` | Publish `BMS_Vmin/Vmax/Tmin/Tmax/ChargeLim`; count down your data timeout (`BMS_Timeout`). |
| `void DecodeCAN(int, uint8_t*)` | none | CAN ISR | Note: **byte pointer**, not `uint32_t*`. |
| `void DeInit()` | none | Before switching away | |

Selected by `BMS_Mode`. CAN bus: `BMSCan`. The BMS does not limit discharge torque in
V2.41A; use `idcmax`/`udcmin` or the inverter's own limits for that.

## Charger hardware (`chargerhw.h`)

On-board AC charger.

| Method | Called from | Contract |
|---|---|---|
| `bool ControlCharge(bool RunChg, bool ACReq)` | `Ms200Task` | Return true while charging should run. `RunChg` combines the user's charge control, timer, plug detect and termination; `ACReq` comes from the charge interface. True (outside Run) sets charge mode, which starts precharge. |
| `void Off()` | State machine while closing down from Off | Tell the charger to stop. |
| `Task1Ms()` | always | |
| `Task10Ms()` | in Charge (always for `Leaf_PDM`) | |
| `Task100Ms()` | always | |
| `Task200Ms()` | in Charge | |
| `DecodeCAN`, `DeInit` | | |

Selected by `chargemodes`. CAN bus: `ChargerCan`.

## Charge interface (`chargerint.h`)

DC fast charge or AC pilot interface (LIM, CHAdeMO, CPC, FOCCCI).

| Method | Called from | Contract |
|---|---|---|
| `bool DCFCRequest(bool RunChg)` | `Ms100Task` | True requests DC charging: sets `chargeModeDC` and (outside Run) charge mode. |
| `bool ACRequest(bool RunChg)` | `Ms100Task` when not DC charging | AC charging allowed by the interface. Default true. |
| `Task10Ms()` | always | |
| `Task100Ms()` | LIM and FOCCCI always; others while DC charging | |
| `Task200Ms()` | CHAdeMO while DC charging; LIM, CPC, FOCCCI and Unused always | |
| `Task1Ms`, `DecodeCAN`, `DeInit` | | |

Selected by `interface`. CAN bus: `LimCan`. CAN3 frames 0x108/0x109 are also delivered
here.

## Heater (`heater.h`)

| Method | Called from | Contract |
|---|---|---|
| `SetPower(uint16_t watts, bool HeatReq)` | `ControlCabHeater()` every 10 ms | Must be called cyclically; `(0, false)` when heating is not allowed. |
| `SetTargetTemperature(float)` | same | Currently always 50 and unused. |
| `GetTemperature()`, `Task100Ms()`, `DecodeCAN`, `DeInit` | | |

Selected by `Heater`. CAN bus: `HeaterCan`. LIN heaters also get `SetLinInterface()`.

## DC-DC (`dcdc.h`)

The base class is "no DC-DC". `Task1Ms/10Ms/100Ms`, `DecodeCAN(int, uint8_t*)`,
`DeInit`. Selected by `DCdc_Type`, bus `DCDCCan`.

## Shifter (`shifter.h`)

A CAN gear lever from another car.

| Method | Called from | Contract |
|---|---|---|
| `bool GetGear(Sgear &g)` | `SelectDirection()` | Same as `Vehicle::GetGear()` but lower priority. |
| `Task1Ms/10Ms/100Ms/200Ms` | Ms1, Ms10, Ms100 (Task200Ms is not called by the core) | Send backlight/position frames. |
| `DecodeCAN` | CAN ISR | |

Selected by `GearLvr`. Uses the **vehicle** bus (`VehicleCan`).

## Who calls what, by task

| | Ms1 | Ms10 | Ms100 | Ms200 |
|---|---|---|---|---|
| Inverter | Task1Ms | Task10Ms (Run), SetTorque, GetMotorSpeed | Task100Ms, temps, voltage, state | |
| Vehicle | Task1Ms | SetRevCounter (Run/Charge), SetTemperatureGauge, Task10Ms, Ready, Start, DashOff (Off) | GetGear, GetCruiseState, GetFrontRearBalance, Task100Ms, Ready | Task200Ms |
| Charger | Task1Ms | Task10Ms (Charge), Off | Task100Ms | ControlCharge, Task200Ms (Charge) |
| Charge interface | Task1Ms | Task10Ms | DCFCRequest, ACRequest, Task100Ms (conditional) | Task200Ms (conditional) |
| BMS | | | Task100Ms | MaxChargeCurrent (Charge) |
| DC-DC | Task1Ms | Task10Ms | Task100Ms | |
| Shifter | Task1Ms | Task10Ms | GetGear, Task100Ms | |
| Heater | | SetPower, SetTargetTemperature | Task100Ms | |
