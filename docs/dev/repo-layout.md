# Repository layout

```text
Stm32-vcu/
├── Makefile              # firmware build; OBJSL lists every object file
├── stm32_vcu.ld          # linker script: 120K flash at 0x08001000, 20K RAM
├── src/                  # project sources (one .cpp per module)
├── include/              # project headers, including the *_prj.h config headers
├── libopeninv/           # git submodule: OpenInverter shared library
├── libopencm3/           # git submodule: STM32 peripheral library
├── test/                 # host-side unit tests (throttle)
├── Parameters/           # example parameter JSON files for real cars
├── Documentation/        # oi-inverter.dbc, regen PDF
├── Hardware/             # schematics, PCB PDFs, Gerbers, header pinout
├── 3d_Printable_Case/    # enclosure models
├── .github/workflows/    # CI build + clang-format check
├── .pre-commit-config.yaml
├── CONTRIBUTING.md, CODEOWNERS, README.md
└── stm32-vcu.cbp         # Code::Blocks project
```

Leftovers you can ignore: `scratchpad.cpp`, `Adruino_ported_450h.txt`,
`e39_can_info.txt` (notes), and a Code::Blocks `.layout` "conflicted copy".

## The core files

| File | Role |
|---|---|
| `src/stm32_vcu.cpp` | `main()`, the four scheduler tasks, the opmode state machine, module selection (`UpdateInv()`, `UpdateVehicle()`, …), `SetCanFilters()`, `CanCallback()` and `Param::Change()`. About 1,450 lines; start reading here. |
| `include/param_prj.h` | The parameter and spot-value list, every option string, and project enums (`InvModes`, `vehicles`, `ChargeModes`, `modes`, …). |
| `src/utils.cpp`, `include/utils.h` | Digital input collection, throttle checks, direction selection, `udc`/shunt processing, SOC, cruise buttons, PWM gauge helpers. |
| `src/throttle.cpp` | Throttle normalisation, regen, ramps, voltage/current/speed/temperature limits. |
| `src/hwinit.cpp`, `include/hwinit.h` | Clocks, RTC, NVIC priorities, SPI, timers, UART setup. |
| `include/hwdefs.h` | Board constants: terminal UART, flash page numbers for params and CAN maps. |
| `include/digio_prj.h` | Every digital pin: name, port, pin, mode. |
| `include/anain_prj.h` | Every analogue input. |
| `src/iomatrix.cpp`, `include/iomatrix.h` | Maps user-selectable I/O functions to physical pins. |
| `include/errormessage_prj.h` | The error list and each error's severity. |
| `src/terminal_prj.cpp` | Terminal command table (`set`, `get`, `save`, `can`, `json`, …). |
| `src/temp_meas.cpp` | Thermistor lookup tables. |

## Module files

Each module is a class in its own `.h`/`.cpp` pair. The base classes are header-only:

| Base class (header) | Implementations |
|---|---|
| `vehicle.h` | `BMW_E31`, `BMW_E39` (also E46), `BMW_E65`, `Can_VAG`, `subaruvehicle`, `V_Classic`, `NoVehicle.h` |
| `inverter.h` | `leafinv`, `GS450H` (also GS300H and Prius Gen3), `outlanderinverter`, `RearOutlanderinverter`, `Can_OI`, `NoInverter.h` |
| `bms.h` | `simpbms`, `daisychainbms`, `leafbms`, `kangoobms` (the base class itself is "no BMS") |
| `chargerhw.h` | `NissanPDM`, `teslaCharger`, `ElconCharger`, `amperacharger`, `outlanderCharger`, `extCharger`, `nocharger.h` |
| `chargerint.h` | `i3LIM`, `chademo`, `CPC`, `Foccci`, `notused.h` |
| `heater.h` | `amperaheater`, `VWCoolantHeater`, `VWAirHeater`, `MGCoolantHeater`, `OutlanderCanHeater`, `noHeater.h` |
| `dcdc.h` | `TeslaDCDC`, `ElconDCDC` (the base class itself is "no DC-DC") |
| `shifter.h` | `F30_Lever`, `E65_Lever`, `JLR_G1`, `JLR_G2`, `no_Lever.h` |

Helpers that are not plug-ins: `isa_shunt`, `bmw_sbox`, `vag_sbox` (shunts and contactor
boxes, static classes), `Can_OBD2` (OBD2 responder), `OutlanderHeartBeat` (keeps
Outlander components awake), `preheater`, `digipot` (two SPI digital pots, used as fuel
gauge drivers), `MCP2515`/`CANSPI` (the third CAN port), `NissLeafMng`, `charger.cpp`.

## libopeninv

The `libopeninv` submodule is shared with other OpenInverter projects. Change it
upstream, not in this repository, unless you are maintaining a fork.

| File | What it gives you |
|---|---|
| `params.h/.cpp` | The `Param::` database built from `PARAM_LIST`. |
| `param_save.h/.cpp` | Save/load parameters to the last flash page with a CRC. |
| `canhardware.h/.cpp` | Abstract CAN interface, user-message registration, receive callbacks. |
| `stm32_can.h/.cpp` | bxCAN driver for CAN1/CAN2: filters, send queue, ISRs. |
| `canmap.h/.cpp` | User-defined CAN TX/RX mapping of parameters. |
| `cansdo.h/.cpp`, `sdocommands.cpp` | CANopen-style SDO server for parameter access. |
| `stm32scheduler.h/.cpp` | Timer-based scheduler for up to four periodic tasks. |
| `terminal.cpp`, `terminalcommands.cpp` | UART command line. |
| `digio.h`, `anain.h` | Pin and ADC abstractions driven by the `*_prj.h` lists. |
| `errormessage.h/.cpp` | Error log with timestamps. |
| `linbus.h/.cpp` | LIN master on USART1 (used by VW heaters). |
| `my_math.h`, `my_fp.h` | `MIN`/`MAX`/`RAMPUP`/`IIRFILTER` macros and fixed-point helpers. |
