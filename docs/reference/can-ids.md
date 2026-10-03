# CAN IDs by module

IDs each driver registers to **receive** (`RegisterUserMessage`) and IDs it **sends**,
taken from the V2.41A source by searching for `RegisterUserMessage(` and `Send(`. IDs
computed at runtime are not listed. Use this to plan buses and avoid collisions: every
selected driver sees every accepted frame from both buses.

All IDs are hexadecimal. IDs above 0x7FF are 29-bit extended.

## Core

| Module | Receives | Sends |
|---|---|---|
| VCU SDO server (CAN1, node 3) | 0x603 | 0x583 |
| `SetCanFilters()` | 0x601 (both buses) | |
| OBD2 responder (`OBD2Can`) | 0x7DF | 0x7E8 |
| CAN map (`CanMapCan`) | user-defined | user-defined, every 100 ms |

## Inverters

| Module | Receives | Sends |
|---|---|---|
| OpenInverter (`Can_OI`) | 0x190 (status), 0x581 (SDO replies from node 1) | 0x3F (control, 10 ms in Run), 0x601 (SDO to node 1 when `ConfigCANOI`) |
| Nissan Leaf (`leafinv`) | 0x1DA, 0x55A | Leaf VCM frames via `NissLeafMng` (0x11A, 0x1D4, 0x50B, …) |
| Outlander front (`outlanderinverter`) | 0x289, 0x299, 0x733 | 0x287, 0x371, 0x285, 0x286 |
| Outlander rear (`RearOutlanderinverter`) | 0x289, 0x299, 0x733 | 0x287, 0x371, 0x285, 0x286 |
| Outlander heartbeat | | 0x285 |
| GS450H / GS300H / Prius | (serial link, no CAN) | |

## Vehicles

| Module | Receives | Sends |
|---|---|---|
| BMW E65 | 0x130, 0x1A0, 0x2FC, 0x480 | 0x0A8, 0x0A9, 0x0AA, 0x0BA, 0x1D0, 0x1D2, 0x332, 0x592 |
| BMW E39 / E46 | 0x153, 0x1F3 | 0x316, 0x329, 0x43B, 0x43F, 0x545 |
| BMW E31 | 0x153 | 0x43B, 0x43F |
| VAG | | 0x280, 0x288, 0x580 |
| Subaru, Classic, None | (no CAN) | |

## Shifters

| Module | Receives | Sends |
|---|---|---|
| BMW F30 lever | 0x197, 0x55E, 0x65E | 0x202, 0x3FD |
| BMW E65 lever | 0x192 | |
| JLR G1 | 0x312 | 0x3F3 |
| JLR G2 | 0x0E0 | 0x02C |

## BMS

| Module | Receives | Sends |
|---|---|---|
| SimpBMS | 0x351, 0x355, 0x356, 0x373 | |
| Daisy-chain BMS | 0x4F1, 0x4F5 | |
| Leaf BMS | 0x1DB, 0x1DC, 0x55B, 0x5BC, 0x5C0, 0x59E, 0x1C2, 0x1ED | |
| Kangoo 33 | 0x155, 0x424, 0x425, 0x7BB | 0x423 |

## Shunts and contactor boxes

| Module | Receives | Sends |
|---|---|---|
| ISA IVT-S | 0x521–0x528 | 0x411 (commands) |
| BMW S-box | 0x200, 0x210, 0x220 | 0x100, 0x300 |
| VW e-box | 0x0BB | 0x0BA, 0x1BFFDA19 |

## Chargers

| Module | Receives | Sends |
|---|---|---|
| Nissan PDM (+ Leaf VCM emulation) | 0x390, 0x679 | 0x11A, 0x1D4, 0x1DB, 0x1DC, 0x1F2, 0x50B, 0x55B, 0x59E, 0x5BC |
| Tesla Gen2 (OI) | 0x108 | 0x102 |
| Outlander | 0x377, 0x389, 0x38A | 0x286 |
| Elcon | 0x18FF50E5 | 0x1806E5F4 |
| `charger.cpp` helper | | 0x109 |

## Charge interfaces

| Module | Receives | Sends |
|---|---|---|
| BMW i3 LIM | 0x272, 0x29E, 0x2B2, 0x2EF, 0x3B4 | 0x03C, 0x112, 0x12F, 0x1A1, 0x2A0, 0x2F1, 0x2FA, 0x2FC, 0x328, 0x330, 0x380, 0x397, 0x3A0, 0x3E8, 0x3E9, 0x3F9, 0x431, 0x432, 0x510, 0x512, 0x51A, 0x540, 0x560 |
| CHAdeMO | 0x108, 0x109 (on CAN3) | 0x100, 0x101, 0x102 (on CAN3) |
| CPC | 0x357 | 0x358 |
| FOCCCI | 0x109, 0x357, 0x596 | 0x358, SDO to its node |
| CAN3 (MCP25625) | 0x108, 0x109 passed to the charge interface | |

## Heaters and DC-DC

| Module | Receives | Sends |
|---|---|---|
| Outlander heater | 0x398 | 0x188 |
| MG coolant heater | 0x2B5, 0x2B6 | 0x2A0 |
| Tesla Gen2 DC-DC | 0x210 | 0x3D8 |
| Elcon DC-DC | 0x1801D08F | 0x18008FD0 |

## Collisions to watch for

- **0x210**: BMW S-box and Tesla DC-DC both receive it.
- **0x357/0x358**: CPC and FOCCCI.
- **0x285/0x286**: Outlander inverter, heartbeat and charger share these by design.
- **0x109**: FOCCCI receives it, `charger.cpp` sends it, and CAN3 forwards it.
- **0x3E9, 0x3F9, 0x1A1**: sent by the i3 LIM driver; also BMW and GM vehicle IDs. Keep
  the LIM off a car bus that uses them.
- **0x601**: registered on both buses by the core; the OpenInverter driver also sends
  SDO requests to 0x601.
