# CAN mapping

CAN mapping sends any parameter or spot value in a CAN frame, or writes one from a
received frame, without changing the firmware. Use it to show VCU values on a display,
log them, or feed the VCU a value from another controller.

The map lives on the bus chosen by `CanMapCan` (read at boot: `save` and `reset` after
changing it).

## Commands

```text
can tx <name> <id> <startbit> <length> <gain> [offset]
can rx <name> <id> <startbit> <length> <gain> [offset]
can print
can del <name>
can clear
save
```

| Field | Meaning |
|---|---|
| `name` | Parameter or spot value name, for example `udc`. |
| `id` | CAN ID, decimal or `0x` hex. IDs above 0x7FF are extended. |
| `startbit` | First bit (0–63), little-endian bit numbering. |
| `length` | Number of bits. Negative selects big-endian (Motorola) byte order. |
| `gain` | Multiplier, may have decimals. TX sends `value × gain`; RX stores `raw × gain`. |
| `offset` | Optional, −128…127, added after the gain. |

Limits: 10 TX IDs, 10 RX IDs, 50 mapped values in total. Mapped TX frames are sent every
**100 ms**.

## Examples

Send `udc` (V) and `idc` (A) for a display, 16 bits each, 0.1 resolution:

```text
can tx udc 0x400 0 16 10
can tx idc 0x400 16 16 10
can tx SOC 0x400 32 8 1
save
```

Feed `udc` from an OpenInverter board's status frame 0x190 (voltage × 10 in bits 8–23,
as set up by `ConfigCANOI`):

```text
can rx udc 0x190 8 16 0.1
```

## Over CAN instead of the terminal

The same map, and every parameter, can be changed over CAN with SDO requests to node 3
on CAN1 (request ID 0x603, reply 0x583). The OpenInverter tools use this.

## Watch out

- RX maps overwrite the value every time the frame arrives. Don't map onto a value a
  driver also writes (for example `udc` with an ISA shunt selected).
- A received `canio` value can act as start, brake, forward and reverse inputs. Once
  `canio` has been non-zero, if nothing at all is received on the **inverter's** bus for
  about a second, `canio` is cleared and `CANTIMEOUT` is posted.
- Frames you transmit must not use IDs another device on that bus uses.
