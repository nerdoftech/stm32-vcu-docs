# Heater, DC-DC and accessories

## Cabin heater

| Parameter | Meaning |
|---|---|
| `Heater` | Heater type: Ampera/Volt, VW coolant (LIN), VW air (LIN), Outlander (CAN), MG coolant. |
| `HeaterCan` | Bus for CAN heaters. LIN heaters use the LIN pin (24). |
| `Control` | `Disable`, `Enable` (heat in Run when requested), or `Timer` (preheat). |
| `HeatPwr` | Power request, W. |

The heater runs when the VCU is in Run with `Control` ≥ 1, or in Preheat. The heat
request comes from an input set to `HeatReq`, or a knob on an analogue input set to
`HeaterPot`:

| `HeatPotDir` | Behaviour |
|---|---|
| `BelowOF` (0) | On when the reading is below `HeatPotOn`. |
| `BelowScale` (1) | As above, and `HeatPercnt` scales from `HeatPotOn` to `HeatPotFull`. |
| `AboveOF` (2) | On when the reading is above `HeatPotOn`. |
| `AboveScale` (3) | As above, with scaling. |

The `HeaterEnable` output turns on with the heater (use it for the heater coolant pump).

## Preheat timer

With `Control = Timer`, the VCU wakes at `Pre_Hrs`:`Pre_Min`, precharges, closes the main
contactor and runs the heater for `Pre_Dur` minutes (`opmode` 5), then shuts down. The
`PreHeatOut` output is on during preheat. The clock is set as described in
[Charging](charging.md#setting-the-clock).

## DC-DC converter

| Parameter | Meaning |
|---|---|
| `DCdc_Type` | `TeslaG2` or `DCDCElcon`. |
| `DCDCCan` | Its bus. |
| `DCSetPnt` | Output voltage (Tesla Gen2). |

The Nissan Leaf PDM includes a DC-DC that is handled by the charger driver.

## Cooling

- `CoolantPump` output: on whenever HV is up (Precharge, Run, Charge, Preheat).
- `CoolingFan` output: on above `FanTemp` (the higher of inverter and charger
  temperature), off 5 °C lower, only in Run and Charge.

## Brake vacuum pump

Set an analogue input to `BrakeVacSensor` and an output to `BrakeVacPump`. In Run, the
pump turns on at `BrkVacHyst` and off at `BrakeVacThresh`. If `BrakeVacThresh` is greater
than `BrkVacHyst`, a higher reading means more vacuum; otherwise the other way round.
Watch `BrkVacVal` while pumping by hand to find the right numbers.

## Lights

- `ReverseLight` output follows reverse selection.
- `BrakeLight` output comes on under strong regen (`RegenBrakeLight`).
- `RunIndication` output is on in Run.

## Gauges

| Gauge | How |
|---|---|
| Tachometer | CAN (vehicle class), or the pulse output on pin 30 (`PumpPWM = TachoOut`). |
| Speedometer | CAN (vehicle class), or the pulse output (`PumpPWM = SpeedoOut`). |
| Temperature | CAN (vehicle class), or a PWM pin set to `PwmTempGauge`. |
| Fuel / SOC | CAN (vehicle class), a PWM pin set to `PwmSocGauge`, or the digital potentiometers (pins 35/36) on BMW E65. |
