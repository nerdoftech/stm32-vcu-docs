# First-time setup

Work through these steps in order. Each one can be checked before any high voltage is
connected, except the last few. Save after each step that works (`save`).

!!! danger "Before you start"
    Keep the HV battery disconnected until step 9. Have the drive wheels off the ground
    for the first drive test. Make sure you can open the HV circuit independently of the
    VCU (a manual disconnect or a fused service plug).

## 1. Power up and connect

- [ ] 12 V permanent to pin 56, ground to pin 55. The on-board LED blinks (it toggles
      every 100 ms while the firmware runs).
- [ ] Connect the Wi-Fi module or a serial adapter and open the web interface or terminal.
- [ ] `get version` shows the expected firmware.
- [ ] `get opmode` shows `0` (Off).
- [ ] `get uaux` is close to your 12 V battery voltage.

## 2. Choose your components

Set these in the **General Setup** group:

| Parameter | Set to |
|---|---|
| `Vehicle` | Your car, or `None` (3). **The default is BMW_E46**, so change it. |
| `Inverter` | Your drive inverter, for example `OpenI` (4) for an OpenInverter board. |
| `BMS_Mode` | Your BMS, or `Off`. |
| `ShuntType` | Your current sensor. Precharge needs one of these (or a Leaf inverter, or a CAN-mapped `udc`). |
| `chargemodes`, `interface` | Your charger and DC charge interface, or `Off`/`Unused`. |
| `DCdc_Type`, `Heater`, `GearLvr` | As fitted. |

Then set each device's bus (`InverterCan`, `VehicleCan`, `BMSCan`, `ShuntCan`,
`ChargerCan`, `LimCan`, `DCDCCan`, `HeaterCan`, `OBD2Can`) to match your wiring.
`CanMapCan` only takes effect after `save` and a reboot. See
[Hardware](hardware.md#can-buses) for how to plan buses.

`save`, then `reset`.

## 3. Check CAN devices

- [ ] Power the inverter's and BMS's 12 V side (the VCU only powers the inverter relay in
      Run, so you may need to power it by hand for this check).
- [ ] With an ISA shunt: `udc`, `idc` and `tmpaux` show sensible numbers. A brand-new
      ISA shunt needs `IsaInit = 1`, `save`, `reset` once, then set `IsaInit` back to 0.
- [ ] With a BMS: `BMS_Vmin`, `BMS_Vmax`, `BMS_Tmin`, `BMS_Tmax` update.
- [ ] With an OpenInverter board: set `ConfigCANOI = 1` once. The VCU programs the
      inverter to send its status frame and sets the parameter back to 0 when done.
      **This erases any CAN mapping already on the inverter board.** Then `tmphs` and
      `tmpm` show the inverter's temperatures.

## 4. Inputs

Watch the spot values while you operate each control:

- [ ] Ignition on/off changes `T15Stat` (with the `None` vehicle, this is pin 15).
- [ ] Start position: `din_start` goes to 1.
- [ ] Brake pedal: `din_brake` goes to 1.
- [ ] Forward/reverse selector: `din_forward`, `din_reverse`. Pick `dirmode` to match
      your switches (momentary buttons, latching switch, or `DefaultForward`).

## 5. Throttle calibration

1. Pedal released: read `pot` (and `pot2` for a dual-channel pedal). Set `potmin` (and
   `pot2min`) **slightly above** this value, about 20–50 counts. The VCU will only start
   when `pot` is below `potmin`, so if `potmin` is at or below the resting value the car
   will not start.
2. Pedal fully pressed: read `pot`/`pot2` and set `potmax`/`pot2max` a little below.
3. Set `potmode` to `DualChannel` if you have two signals.
4. Press slowly through the travel while watching `pot`; it should change smoothly
   without jumps.

Use a pedal whose signal **rises** as you press. The start interlock assumes it.

With an OpenInverter drive unit, also calibrate the pedal on the inverter board: the VCU
forwards raw pedal readings and the inverter does the mapping.

## 6. Limits

Set these before any HV is connected:

| Parameter | Typical starting point |
|---|---|
| `udcsw` | About 90–95 % of nominal pack voltage (the precharge-complete threshold). Some shunts and BMSs set it automatically. |
| `udclim` | A little above full-charge pack voltage. Above this the VCU shuts down. |
| `udcmin` | Pack voltage at which you want drive power to start fading. 0 disables. |
| `idcmax`, `idcmin` | Pack discharge and regen current limits (needs a shunt). |
| `revlim` | Motor speed limit. |
| `tmphsmax`, `tmpmmax` | Inverter and motor temperature limits. |
| `throtmax`, `throtmaxRev` | Start low (for example 30 % and 15 %) for first tests. |
| `regenmax`, `regenBrake` | Start with small negative values, for example −5. |

## 7. Outputs

Assign the general-purpose outputs ([General purpose I/O](gpio.md)):

- [ ] `NegContactor` on whichever output drives your negative contactor (default:
      Out 2).
- [ ] `CoolantPump`, `CoolingFan`, `BrakeLight`, `ReverseLight`, `HeaterEnable` as
      needed.

With the HV battery still disconnected, you can watch contactors click through the
sequence: ignition on, start. Precharge will close and then fail after 5 s (no voltage).
That's expected here and confirms the wiring order. `opmode` shows `3` (PchFail); key off
to reset.

## 8. Precharge check

- [ ] Measure the precharge resistor and estimate the time constant with your inverter's
      capacitance. Precharge must reach `udcsw` within 5 s or it fails.

## 9. First HV power-up

1. Connect HV. Wheels off the ground.
2. Ignition on, start. `opmode` goes 2 (Precharge) then 1 (Run). `udc` rises to pack
   voltage.
3. If it goes to 3 (PchFail), see [Troubleshooting](troubleshooting.md#precharge-fails).

## 10. First drive

1. Select forward. `dir` shows 1.
2. Gently press the pedal. `potnom` follows the pedal; `speed` follows the motor.
3. Check the motor turns the right way. With an OpenInverter board, reverse rotation on
   the inverter. With other inverters see `reversemotor` (only honoured for the rear
   Outlander motor).
4. Check reverse, then regen on lift-off and with the brake.
5. Raise `throtmax` and regen in steps.

`save` the final configuration and keep a JSON backup.
