# Connector pinout

ZombieVerter V1 main connector, cable side, from `Hardware/Zombie/ZOM_V1_HEADER_PINOUT.ods` in
the firmware repository. Wire colours refer to the loom described in that file.

!!! warning "Check your board revision"
    This list was written for the V1 board. Confirm every pin against the schematic for
    your revision (`Zom_V1_3.pdf` for V1.3) before wiring. The source file also notes that
    some connections were unrouted on pre-release hardware (workaround: use a GP analogue
    input for temperature and the GP 12 V input for the brake light).

| Pin | Wire | Function | Notes |
|---|---|---|---|
| 1 | green | RS232 Rx | Rs232 comms from Usart 3 on mcu. May be used for remote wifi locating or direct serial terminal comms. |
| 2 | purple | RS232 Tx | Rs232 comms from Usart 3 on mcu. May be used for remote wifi locating or direct serial terminal comms. |
| 3 | white | GP Out 3 | General purpose protected Low side switched output from PD13 |
| 4 | blue | GP Out 2 | General purpose protected Low side switched output from PD14 |
| 5 | purple | PWM 3 | General purpose pwm output. Push/pull driven to +12v/gnd. Timer 3 Chan 3 PB0. |
| 6 | orange | PWM 2 | General purpose pwm output. Push/pull driven to +12v/gnd. Timer 3 Chan 2 PA7. |
| 7 | orange | PWM 1 | General purpose pwm output. Push/pull driven to +12v/gnd. Timer 3 Chan 1 PA6. |
| 8 | orange | Analog 2 in | General purpose analog input 0-5v.PC3. |
| 9 | orange | Analog 1 in | General purpose analog input 0-5v.PC2. |
| 10 | orange | DAC 2 | Digital to Analog converter output 1. Amplified via TDA2822 to max 12v level. PA5. |
| 11 | brown | DAC 1 | Digital to Analog converter output 1. Amplified via TDA2822 to max 12v level. PA4. |
| 12 | blue | MG2 Temp - | Motor 2 temp sensor. Can be used for Toyota hybrid transmissions or as a general purpose motor temp input.PC4. |
| 13 | yellow | MG2 Temp + | Motor 2 temp sensor. Can be used for Toyota hybrid transmissions or as a general purpose motor temp input.PC4. |
| 14 | green | MG1 Temp - | Motor 2 temp sensor. Can be used for Toyota hybrid transmissions or as a general purpose motor temp input.PC5. |
| 15 | purple | Ignition T15 In | Terminal 15 ignition on 12v input. Tells the vcu the key is in the on position.PD6. |
| 16 | purple | REQ- | Toyota Hybrid inverter serial comms.PE6. |
| 17 | yellow | REQ+ | Toyota Hybrid inverter serial comms.PE6. |
| 18 | yellow | CLK- | Toyota Hybrid inverter serial comms clock. Timer 2 Channel 2. PA1. |
| 19 | green | CLK+ | Toyota Hybrid inverter serial comms clock. Timer 2 Channel 2. PA1. |
| 20 | green | MTH- | Toyota Hybrid inverter serial comms.Usart 2. |
| 21 | brown | MTH+ | Toyota Hybrid inverter serial comms.Usart 2. |
| 22 | brown | HTM- | Toyota Hybrid inverter serial comms.Usart 2. |
| 23 | white | HTM+ | Toyota Hybrid inverter serial comms.Usart 2. |
| 24 | green | LIN | Local interconnect network. Usart 1. |
| 25 | blue | CAN EXT 3L | CAN 3 interface. Choice of HS/SW/FT modes via jumpers. |
| 26 | yellow | CAN EXT 3H | CAN 3 interface. Choice of HS/SW/FT modes via jumpers. |
| 27 | red | CAN EXT 2L | CAN 2 interface. HS only. Terminated with 120R by default. |
| 28 | red | CAN EXT 2H | CAN 2 interface. HS only. |
| 29 | white | MG1 Temp + | Motor 2 temp sensor. Can be used for Toyota hybrid transmissions or as a general purpose motor temp input.PC5. |
| 30 | brown | Oil Pump PWM | PWM for Toyota hybrid oil pump speed control. Weak pullup to +12v via 1k. May be used as a general low power low side pwm output. Timer 1 channel 1 PE9. |
| 31 | brown | GP Out 1 | General purpose protected Low side switched output from PD15. |
| 32 | green | Inverter Power | Low side switched output for controlling drive inverter power relay.PA8. |
| 33 | yellow | Main Contactor | Low side switched output for controlling main HV contactor.PC7. |
| 34 | blue | Precharge | Low side switched output for controlling main HV precharge contactor.PC6. |
| 35 | black | Pot 2 | Output from AD5160 0-5k digital potentiometer. Controlled via SPI3. |
| 36 | purple | Pot 1 | Output from AD5160 0-5k digital potentiometer. Controlled via SPI3. |
| 37 | green | Trans SP | Protected high side switched +12v supply to Toyota hybrid transmission. May be used as a GP output if desired.PD12. |
| 38 | blue | Trans SL2- | Protected low side switched +12v supply to Toyota hybrid transmission. May be used as a GP output if desired.PC8. |
| 39 | yellow | Trans SL1- | Protected low side switched +12v supply to Toyota hybrid transmission. May be used as a GP output if desired.PC9. |
| 40 | green | Trans PB3 | Input from Toyota hybrid transmission gear position indicator. May be used as a GP input if desired.PE5. |
| 41 | red | Trans PB2 | Input from Toyota hybrid transmission gear position indicator. May be used as a GP input if desired.PE4. |
| 42 | black | Trans PB1 | Input from Toyota hybrid transmission gear position indicator. May be used as a GP input if desired.PE3. |
| 43 | black | CAN EXT L | CAN 1 interface. HS only. Terminated with 120R by default. |
| 44 | black | CAN EXT H | CAN 1 interface. HS only. |
| 45 | black | Throttle Ground | Hall effect throttle pedal/position sensor ground. |
| 46 | black | Throttle 2 | Hall effect throttle pedal/position sensor channel 2. PC1. |
| 47 | black | Throttle 1 | Hall effect throttle pedal/position sensor channel 1. PC0. |
| 48 | black | +5v Throttle | Hall effect throttle pedal/position sensor +5v supply. |
| 49 | black | Brake input | Brake light switch input.PA15. |
| 50 | orange | GP12v Input | General purpose 12v digital input.PD4. |
| 51 | 2 core brown | Hvrequest | High voltage switch on/off request digital 12v input. Typically used by a charger to request HV activation.PD5. |
| 52 | 2 core blue | Start | Start signal momentary digital 12v input. Typically connected to the start position of a traditional ignition switch. |
| 53 | 2 core brown | Reverse Direction | Pull to +12v for reverse direction |
| 54 | 2 core blue | Forward Direction | Pull to +12v for forward direction |
| 55 | Purple | Ground | Connect to vehicle chassis ground |
| 56 | Green | Permanent +12V | Connect to permanent +12v via 5amp fuse |

## Firmware names for these pins

| Pin | Function | Firmware object | Configured by |
|---|---|---|---|
| 15 | Ignition T15 | `DigIo::t15_digi` (PD6) | fixed |
| 52 | Start | `DigIo::start_in` (PD7) | fixed |
| 49 | Brake | `DigIo::brake_in` (PA15) | fixed |
| 54 / 53 | Forward / Reverse | `DigIo::fwd_in` (PB4) / `rev_in` (PB3) | `dirmode` |
| 47 / 46 | Throttle 1 / 2 | `AnaIn::throttle1` (PC0) / `throttle2` (PC1) | `potmin`… |
| 34 | Precharge | `DigIo::prec_out` (PC6) | fixed |
| 33 | Main contactor | `DigIo::dcsw_out` (PC7) | fixed |
| 32 | Inverter power | `DigIo::inv_out` (PA8) | fixed |
| 31, 4, 3 | GP Out 1–3 | `gp_out1..3` (PD15, PD14, PD13) | `Out1Func`…`Out3Func` |
| 39, 38 | Trans SL1, SL2 | `SL1_out`, `SL2_out` (PC9, PC8) | `SL1Func`, `SL2Func` |
| 7, 6, 5 | PWM 1–3 | `PWM1..3` (PA6, PA7, PB0) | `PWM1Func`…`PWM3Func` |
| 50 | GP 12 V in | `gp_12Vin` (PD4) | `GP12VInFunc` |
| 51 | HV request | `HV_req` (PD5) | `HVReqFunc` |
| 42, 41, 40 | Trans PB1–3 | `gear1_in..gear3_in` (PE3–PE5) | `PB1InFunc`…`PB3InFunc` |
| 9, 8 | Analogue 1, 2 | `GP_analog1`, `GP_analog2` (PC2, PC3) | `GPA1Func`, `GPA2Func` |
| 30 | Oil pump PWM | TIM1 CH1 (PE9) | `PumpPWM` |
| 36, 35 | Pot 1, 2 | AD5160 on SPI3 | `DigiPot1Step`, `DigiPot2Step` |
| 24 | LIN | USART1 | heater selection |
| 1, 2 | RS232 | USART3 | `UseRS232` |
