# Module catalog

Every module the V2.41A firmware can select, with the parameter value that selects it,
the source file, and the CAN bus parameter it follows. For the CAN IDs each module
receives and sends, see [CAN IDs by module](../reference/can-ids.md).

## Vehicles (`Vehicle`, bus `VehicleCan`)

| Value | Option | Class / file | Notes |
|---|---|---|---|
| 0 | `BMW_E46` | `BMW_E39` with `SetE46(true)` / `BMW_E39.cpp` | Same class as E39, E46 variant of the dash frames. |
| 1 | `BMW_E6x+` | `BMW_E65` / `BMW_E65.cpp` | Reads CAS (ignition, start button), speed, lock state; sends ABS/DSC, engine data, gear display, warning lights; fuel gauge via digital pots. Good reference for a CAN-heavy class. |
| 2 | `Classic` | `V_Classic` / `V_Classic.cpp` | No CAN. Tacho/speedo pulse output on the oil pump pin, PWM temperature and SOC gauges. Ignition from the T15 pin. |
| 3 | `None` (shown) | none | Matches no `case`; see [Known issues](known-issues.md#vehicle-none-is-3-in-the-ui-but-4-in-code). |
| 4 | (internal None) | `NoVehicle` / `NoVehicle.h` | `Ready()` = T15 pin, `Start()` = start input. The boot default. |
| 5 | `BMW_E39` | `BMW_E39` / `BMW_E39.cpp` | E39 dash and DME frames; `Transmission` chooses manual/auto. |
| 6 | `VAG` | `Can_VAG` / `Can_VAG.cpp` | Mid-2000s VW/Audi powertrain frames (0x280, 0x288, 0x580). |
| 7 | `Subaru` | `SubaruVehicle` / `subaruvehicle.cpp` | Analogue gear and cruise selectors read from GP analogue inputs; PWM gauges. A good example of `GetGear()`, `GetCruiseState()` and `GetFrontRearBalance()`. |
| 8 | `BMW_E31` | `BMW_E31` / `BMW_E31.cpp` | BMW 8-series. |

## Inverters (`Inverter`, bus `InverterCan`)

| Value | Option | Class / file | Notes |
|---|---|---|---|
| 0 | `None` | `NoInverterClass` / `NoInverter.h` | Returns zeros. Bench testing. |
| 1 | `Leaf_Gen1` | `LeafINV` / `leafinv.cpp` | Nissan Leaf inverter over CAN. Publishes its DC voltage to `udc`, so it can serve as the precharge reference without a shunt. |
| 2 | `GS450H` | `GS450HClass` / `GS450H.cpp` | Lexus GS450h inverter and gearbox over the synchronous serial link (USART2 + DMA, TIM2 clock). Also runs the oil pump and gear selection. |
| 3 | `UserCAN` | none | No case in `UpdateInv()`; selecting it leaves the previous inverter active. |
| 4 | `OpenI` | `Can_OI` / `Can_OI.cpp` | Any OpenInverter board (for example a Tesla LDU or SDU with an OI controller). Sends 0x3F, reads 0x190, can configure the board over SDO (`ConfigCANOI`). See [Throttle pipeline](throttle.md#openinverter-is-different). |
| 5 | `Prius_Gen3` | `GS450HClass` with `SetPrius()` | Toyota Prius/Yaris/Auris Gen3 inverter. |
| 6 | `Outlander` | `OutlanderInverter` / `outlanderinverter.cpp` | Mitsubishi Outlander front motor. Turns on the Outlander heartbeat. |
| 7 | `GS300H` | `GS450HClass` with `SetGS300H()` | |
| 8 | `RearOutlander` | `RearOutlanderInverter` / `RearOutlanderinverter.cpp` | Outlander rear motor. The only inverter that honours `reversemotor`. |

## BMS (`BMS_Mode`, bus `BMSCan`)

| Value | Option | Class / file | Notes |
|---|---|---|---|
| 0 | `Off` | `BMS` (base) | No limits. |
| 1 | `SimpBMS` | `SimpBMS` / `simpbms.cpp` | SimpBMS protocol (0x351, 0x355, 0x356, 0x373). Without a shunt it also provides `udc2`, `idc` and `udcsw`. |
| 2, 3 | `TiDaisychainSingle/Dual` | `DaisychainBMS` / `daisychainbms.cpp` | Daisy-chain BMS on 0x4F1 (and 0x4F5 for a second unit). |
| 4 | `LeafBms` | `LeafBMS` / `leafbms.cpp` | Nissan Leaf LBC. |
| 5 | `RenaultKangoo33` | `KangooBMS` / `kangoobms.cpp` | Renault Kangoo 33 kWh pack. |

## AC chargers (`chargemodes`, bus `ChargerCan`)

| Value | Option | Class / file |
|---|---|---|
| 0 | `Off` | `noCharger` / `nocharger.h` |
| 1 | `EXT_DIGI` | `extCharger` / `extCharger.cpp` (enables an external charger with the `OBCEnable` output) |
| 2 | `Volt_Ampera` | `amperaCharger` / `amperacharger.cpp` |
| 3 | `Leaf_PDM` | `NissanPDM` / `NissanPDM.cpp` (Leaf charger + DC-DC; also emulates the Leaf VCM via `NissLeafMng`) |
| 4 | `TeslaOI` | `teslaCharger` / `teslaCharger.cpp` (Tesla Gen2 charger with OpenInverter control board) |
| 5 | `Out_lander` | `outlanderCharger` / `outlanderCharger.cpp` |
| 6 | `Elcon` | `ElconCharger` / `ElconCharger.cpp` |

## Charge interfaces (`interface`, bus `LimCan`)

| Value | Option | Class / file |
|---|---|---|
| 0 | `Unused` | `notused` / `notused.h` |
| 1 | `i3LIM` | `i3LIMClass` / `i3LIM.cpp` (BMW i3 charging module: CCS DC and AC pilot) |
| 2 | `Chademo` | `FCChademo` / `chademo.cpp` |
| 3 | `CPC` | `CPCClass` / `CPC.cpp` |
| 4 | `Foccci` | `FoccciClass` / `Foccci.cpp` (open-source CCS controller, configurable over SDO) |

## Heaters (`Heater`, bus `HeaterCan`)

| Value | Option | Class / file |
|---|---|---|
| 0 | `None` | `noHeater` |
| 1 | `Ampera` | `AmperaHeater` / `amperaheater.cpp` |
| 2 | `VWCoolant` | `vwCoolantHeater` / `VWCoolantHeater.cpp` (LIN) |
| 3 | `VWAir` | `vwAirHeater` / `VWAirHeater.cpp` (LIN) |
| 4 | `OutlanderCan` | `OutlanderCanHeater` / `OutlanderCanHeater.cpp` |
| 5 | `MGCoolant` | `mgCoolantHeater` / `MGCoolantHeater.cpp` |
| 6 | `PWM` | none in `UpdateHeater()` |

## DC-DC converters (`DCdc_Type`, bus `DCDCCan`)

| Value | Option | Class / file |
|---|---|---|
| 0 | `NoDCDC` | `DCDC` (base) |
| 1 | `TeslaG2` | `TeslaDCDC` / `TeslaDCDC.cpp` |
| 2 | `DCDCElcon` | `ElconDCDC` / `ElconDCDC.cpp` |

## Shifters (`GearLvr`, bus `VehicleCan`)

| Value | Option | Class / file |
|---|---|---|
| 0 | `None` | `Shifter` (base) |
| 1 | `BMW_F30` | `F30_Lever` / `F30_Lever.cpp` |
| 2 | `JLR_G1` | `JLR_G1` / `JLR_G1.cpp` |
| 3 | `JLR_G2` | `JLR_G2` / `JLR_G2.cpp` |
| 4 | `BMW_E65` | `E65_Lever` / `E65_Lever.cpp` |

## Shunts and contactor boxes (`ShuntType`, bus `ShuntCan`)

Static helper classes, not plug-ins. `SetCanFilters()`, `CanCallback()` and
`ProcessUdc()` switch on `ShuntType` directly.

| Value | Option | File | Notes |
|---|---|---|---|
| 1 | `ISA` | `isa_shunt.cpp` | Isabellenhütte IVT-S: current, three voltages, power, kWh, Ah, temperature. `IsaInit` sends its setup. |
| 2 | `SBOX` | `bmw_sbox.cpp` | BMW i3 contactor box: measures and switches contactors over CAN. |
| 3 | `VAG` | `vag_sbox.cpp` | VW e-box. |
| 4 | `ISA_udcsw` | `isa_shunt.cpp` | ISA plus automatic `udcsw` = U2 − 20 V. |
