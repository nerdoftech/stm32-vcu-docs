# Adding other modules

Inverters, BMSs, chargers, charge interfaces, heaters, DC-DC converters and shifters all
plug in the same way as a vehicle. The pattern is always:

1. Write a class deriving from the base class ([Module interfaces](interfaces.md)).
2. Add an option value: raise the selector parameter's `max`, extend its option string,
   and extend the enum in `param_prj.h`.
3. Create a static instance in `stm32_vcu.cpp` and add a `case` to the matching
   `UpdateX()` function.
4. Add the object to `OBJSL` in the `Makefile`.
5. Build, run `pre-commit`, run the unit tests.

The table tells you which names to touch for each kind.

| Kind | Base class | Selector param | Option string | Enum | Update function | Bus param |
|---|---|---|---|---|---|---|
| Vehicle | `Vehicle` | `Vehicle` | `VEHMODES` | `vehicles` | `UpdateVehicle()` | `VehicleCan` |
| Inverter | `Inverter` | `Inverter` | `INVMODES` | `InvModes` | `UpdateInv()` | `InverterCan` |
| BMS | `BMS` | `BMS_Mode` | `BMSMODES` | `BMSModes` | `UpdateBMS()` | `BMSCan` |
| AC charger | `Chargerhw` | `chargemodes` | `CHGMODS` | `ChargeModes` | `UpdateCharger()` | `ChargerCan` |
| Charge interface | `Chargerint` | `interface` | `CHGINT` | `ChargeInterfaces` | `UpdateChargeInt()` | `LimCan` |
| Heater | `Heater` | `Heater` | `HTTYPE` | `HeatType` | `UpdateHeater()` | `HeaterCan` |
| DC-DC | `DCDC` | `DCdc_Type` | `DCDCTYPES` | `DCDCModes` | `UpdateDCDC()` | `DCDCCan` |
| Shifter | `Shifter` | `GearLvr` | `SHIFTERS` | `ShifterModes` | `UpdateShifter()` | `VehicleCan` |

## Inverter specifics

- `SetTorque()` is called **every 10 ms in every mode**, with 0 outside Run. Send your
  command frame from there (or from `Task10Ms()`, which only runs in Run), and make sure
  a 0 request also means "no torque" to the inverter.
- `GetMotorSpeed()` must be signed. The core uses the sign for regen direction and
  `DirChangeRpm`.
- If the inverter reports DC-link voltage reliably and you want it to serve as the
  precharge reference, write it to `Param::udc` (as `leafinv.cpp` does) and decide
  whether the inverter needs power during precharge. The state machine powers it in
  precharge for every inverter except `OpenI`.
- Implement `DeInit()` if you reconfigure pins, timers or UARTs, so switching to another
  inverter at runtime leaves the hardware clean.
- Throttle derates and ramps happen before `SetTorque()`. If your inverter does its own
  throttle processing (like OpenInverter), document which side's parameters the user
  must tune.

## BMS specifics

- `DecodeCAN()` receives `uint8_t *data`, not `uint32_t *`.
- Implement a data timeout: reset a counter in `DecodeCAN()`, count it down in
  `Task100Ms()`, and return 0 from `MaxChargeCurrent()` when it expires. Use
  `BMS_Timeout` for the period. `simpbms.cpp` is a clean example.
- Publish `BMS_Vmin`, `BMS_Vmax`, `BMS_Tmin`, `BMS_Tmax` and `BMS_ChargeLim` from
  `Task100Ms()`.
- Gate charging on `BMS_VmaxLimit`, `BMS_VminLimit`, `BMS_TmaxLimit` and
  `BMS_TminLimit`.
- If no shunt is fitted, you can also publish pack voltage to `udc2` and current to
  `idc`, and set `udcsw` from pack voltage, like SimpBMS does.
- The core does not use the BMS to limit drive torque. If you need that, adjust
  `idcmax`/`idcmin` or add a call in `utils::ProcessThrottle()`, and say so in your PR.

## Charger specifics

- `ControlCharge(RunChg, ACReq)` decides whether the VCU enters charge mode. Return
  `RunChg && ACReq && <plug/pilot ok>` in the simplest case.
- `Off()` is called repeatedly while the VCU is shutting down from Off; make it
  idempotent.
- `Task10Ms()` and `Task200Ms()` only run in Charge mode (except `Leaf_PDM`).
- Respect `Pwrspnt`, `Voltspnt` and `BMS_ChargeLim`.

## Adding a shunt type

Shunts are not plug-ins. Adding one means editing `ShuntType`'s option string and max,
`SetCanFilters()`, `CanCallback()`, `utils::ProcessUdc()` and, if it switches contactors,
`Ms10Task()`. Follow how `ShuntType == 2` (`SBOX`) is handled.

## Adding something that is not a plug-in

For a one-off feature (a new output function, a pump controller), prefer:

- a new [I/O matrix](io.md#adding-an-io-function) function, if it is "turn a pin on when
  X";
- a small static class called from one of the existing tasks in `stm32_vcu.cpp`, like
  `Preheater`, if it needs state.

Remember the scheduler is full: put periodic work in an existing task.
