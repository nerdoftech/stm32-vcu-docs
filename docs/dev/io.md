# I/O, ADC and PWM

## Pin lists

Pins are declared once, as X-macro lists, and libopeninv generates `DigIo::name` and
`AnaIn::name` objects from them.

### Digital pins (`include/digio_prj.h`)

| Name | Port/pin | Mode | Use |
|---|---|---|---|
| `HV_req` | PD5 | input | HV request input |
| `start_in` | PD7 | input | Start |
| `brake_in` | PA15 | input | Brake light switch |
| `fwd_in` | PB4 | input | Forward |
| `rev_in` | PB3 | input | Reverse |
| `t15_digi` | PD6 | input | Ignition (T15) |
| `gp_12Vin` | PD4 | input | GP 12 V input (I/O matrix) |
| `gear1_in`/`gear2_in`/`gear3_in` | PE3/PE4/PE5 | input, inverted | Toyota PB1–3 (I/O matrix) |
| `dcsw_out` | PC7 | output | Main contactor |
| `prec_out` | PC6 | output | Precharge contactor |
| `inv_out` | PA8 | output | Inverter power relay |
| `gp_out1`/`gp_out2`/`gp_out3` | PD15/PD14/PD13 | output | GP Out 1–3 (I/O matrix) |
| `SL1_out`/`SL2_out` | PC9/PC8 | output | Toyota SL1/SL2 (I/O matrix) |
| `SP_out` | PD12 | output | Toyota SP supply |
| `PWM1`/`PWM2`/`PWM3` | PA6/PA7/PB0 | output or TIM3 | PWM 1–3 (I/O matrix) |
| `led_out` | PE2 | output | Status LED, toggled every 100 ms |
| `req_out` | PE6 | output | Toyota REQ |
| `sw_mode0`/`sw_mode1` | PD9/PD8 | output | CAN3 transceiver mode |
| `lin_wake`/`lin_nslp` | PD10/PD11 | output | LIN transceiver |
| `pot1_cs`/`pot2_cs` | PD3/PD2 | output | Digital pot chip selects |
| `mcp_cs`/`mcp_sby` | PB12/PE14 | output | MCP25625 (CAN3) |
| `CANEN`/`CANSBY` | PD0/PD1 | output | CAN1 transceiver enable/standby (V1.3) |
| `dummypin` | PE7 | input, pull-down | Target for unassigned I/O matrix functions |

### Analogue inputs (`include/anain_prj.h`)

12-sample averaging, 7.5-cycle sample time. 12-bit, 0–4095.

| Name | Pin | Use |
|---|---|---|
| `throttle1`, `throttle2` | PC0, PC1 | Pedal channels |
| `uaux` | PB1 | 12 V supply sense (`uaux` = ADC / 210) |
| `GP_analog1`, `GP_analog2` | PC2, PC3 | Analogue 1/2 (I/O matrix) |
| `MG1_Temp`, `MG2_Temp` | PC5, PC4 | Motor temperature (Toyota) |
| `dummyAnal` | PC11 | Target for unassigned analogue functions |

## The I/O matrix

Some pins have fixed jobs. The rest are assigned by parameter, through `IOMatrix`:

```cpp
IOMatrix::GetPinOut(IOMatrix::COOLINGFAN)->Set();
if (IOMatrix::GetPinIn(IOMatrix::HVIL) != &DigIo::dummypin) { ... }
int v = IOMatrix::GetAnaloguePin(IOMatrix::VAC_SENSOR)->Get();
```

A function that no pin is assigned to returns `DigIo::dummypin` (or `AnaIn::dummyAnal`),
so code can call `Set()`/`Get()` on it unconditionally. Check against `&DigIo::dummypin`
when "not configured" should mean "skip".

`AssignFromParams()` maps parameters to pins **by position**:

| Parameter | Pin object |
|---|---|
| `Out1Func`, `Out2Func`, `Out3Func` | `gp_out1`, `gp_out2`, `gp_out3` |
| `SL1Func`, `SL2Func` | `SL1_out`, `SL2_out` |
| `PWM1Func`, `PWM2Func`, `PWM3Func` | `PWM1`, `PWM2`, `PWM3` |
| `GP12VInFunc`, `HVReqFunc` | `gp_12Vin`, `HV_req` (inputs) |
| `PB1InFunc`, `PB2InFunc`, `PB3InFunc` | `gear1_in`, `gear2_in`, `gear3_in` (inputs) |
| `GPA1Func`, `GPA2Func` | `GP_analog1`, `GP_analog2` |

It runs from `Param::Change()`, so after every parameter change.

### Adding an I/O function

1. Append to `IOMatrix::pinoutfuncs` (or `pininfuncs`/`analoguepinfuncs`) **before**
   `LAST_OUT`. Order matters: the enum value is the parameter value.
2. Append the matching `N=Name` to `PINOUTFUNCS` (or `PININFUNCS`/`APINFUNCS`) in
   `param_prj.h`.
3. Raise the `max` of the `...Func` parameters that should offer it.
4. Use it with `IOMatrix::GetPinOut(IOMatrix::YOURFUNC)`.

See [Known issues](known-issues.md#io-matrix-reset-loops) before relying on unassigned
functions being reset correctly.

## PWM

### TIM3: PWM 1–3

The three PWM pins share TIM3 (channels 1–3). `tim3_setup()` runs at boot and whenever a
PWM parameter changes.

- If any PWM pin's function is `CpSpoof`, `GS450pump`, `PwmTempGauge` or
  `PwmSocGauge`, TIM3 is forced to **1 kHz** (prescaler 17, period 3999). Duty is then
  set in code as `percent * 40`. The `Tim3_*` parameters are overwritten to show this.
- Otherwise TIM3 uses `Tim3_Presc`, `Tim3_Period` and `Tim3_x_OC` directly, for pins set
  to `PwmTim3`.
- A PWM pin with any other function is a plain digital output.

Because the timer is shared, all three pins run at the same frequency.

### TIM1: Oil Pump PWM pin

PE9 (TIM1 channel 1) serves the Toyota oil pump, or a tacho/speedo pulse output
selected by `PumpPWM`. `utils::SpeedoStart()` sets the timer up and
`utils::SpeedoSet(rpm)` sets a 50 % square wave whose period is
`66 000 000 / (rpm × TachoPPR)` timer ticks, so frequency is proportional to speed. `V_Classic` and `SubaruVehicle` use it.

### Digital potentiometers

Two AD5160 pots on SPI3 (`DigiPot1Step`, `DigiPot2Step`, 0–255) appear on connector pins
35/36. `BMW_E65::SetFuelGauge()` uses them to drive a resistive fuel gauge from SOC.
`Ms100Task` pushes the current step values out every cycle.

## Timers in use

| Timer | Use |
|---|---|
| TIM1 | Oil pump / tacho / speedo pulse output |
| TIM2 | Toyota sync-serial clock |
| TIM3 | PWM 1–3 |
| TIM4 | Scheduler |
| RTC | 1 s clock, `uptime`, charge/preheat timers |
| IWDG | Watchdog, fed in `Ms100Task` |
