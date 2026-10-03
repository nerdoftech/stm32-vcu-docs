# Parameter system

Everything the user can configure, and almost everything the firmware measures, is an
entry in one table generated at compile time from `include/param_prj.h`. The same table
backs the web interface, the terminal, CAN SDO, CAN mapping and flash storage.

## Declaring parameters

`param_prj.h` defines `PARAM_LIST` as a chain of X-macros:

```cpp
//              category     name      unit      min  max  default id
PARAM_ENTRY(CAT_SETUP,    Vehicle,  VEHMODES, 0,   8,   0,      6)
PARAM_ENTRY(CAT_THROTTLE, potmin,   "dig",    0,   4095, 0,     7)
VALUE_ENTRY(opmode, OPMODES, 2002)
VALUE_ENTRY(udc,    "V",     2006)
```

| Field | Meaning |
|---|---|
| `category` | Grouping in the web UI, a `CAT_...` string macro. |
| `name` | Becomes the enum constant `Param::name` and the name used by `set`/`get`. |
| `unit` | A string like `"V"`, or a macro holding an option list like `"0=Off, 1=On"`. The web UI shows a drop-down when the unit contains `=`. |
| `min`, `max`, `default` | Range and default. Range is enforced only by `Param::Set()` (terminal, web UI, SDO), not by `SetInt()`/`SetFloat()`. |
| `id` | **Persistent key.** Saved values are matched by ID, and SDO can address a parameter by ID (index `0x2100` + high byte, sub-index = low byte). Never reuse or renumber an ID. |

`libopeninv/include/params.h` expands the list into `enum Param::PARAM_NUM { Inverter,
Vehicle, ..., PARAM_LAST, PARAM_INVALID }`, so each name is also an index.

### Ordering rules

The header comment states the required order:

1. Saveable parameters (`PARAM_ENTRY`, non-zero ID).
2. Temporary parameters.
3. Display values (`VALUE_ENTRY`).

Some code also depends on the **relative order** of entries because it does arithmetic
on `PARAM_NUM`:

- `IOMatrix::AssignFromParams()` walks from `FIRST_IO_PARAM` (`Out1Func`) through the
  next ten entries, and from `SEC_IO_PARAM` (`PB1InFunc`) through the next three.
  `Out1Func, Out2Func, Out3Func, SL1Func, SL2Func, PWM1Func, PWM2Func, PWM3Func,
  GP12VInFunc, HVReqFunc` must stay adjacent and in this order, as must
  `PB1InFunc, PB2InFunc, PB3InFunc`.
- `AssignFromParamsAnalogue()` does the same from `FIRST_AI_PARAM` (`GPA1Func`,
  `GPA2Func`).

Do not insert a new entry inside those runs.

### IDs

The header tracks the next free IDs in comments:

```cpp
// Next param id (increase when adding new parameter!): 157
// Next value Id: 2124
```

Take the next number and bump the comment in the same commit. Upstream and your fork can
collide here; if you maintain a fork, pick IDs from a high range (for example 500+ for
parameters, 2500+ for values) to stay clear of future upstream additions.

## Reading and writing from code

```cpp
int   mode = Param::GetInt(Param::opmode);
float udc  = Param::GetFloat(Param::udc);
bool  brk  = Param::GetBool(Param::din_brake);

Param::SetInt(Param::Veh_Speed, kph);      // no range check, no callback
Param::SetFloat(Param::tmphs, temp);
```

Values are stored as 32-bit fixed point (`s32fp`, 5 fractional bits in libopeninv's
default configuration), so very small fractions round. `GetInt()` truncates.

`SetInt`/`SetFloat` neither check the range nor call `Param::Change()`. That is what you
want for spot values. If your code changes a *setting* and other code depends on the
`Change()` side effects, call `Param::Change(Param::thatParam)` yourself.

## The change callback

`Param::Change(PARAM_NUM)` is defined in `stm32_vcu.cpp` and is called by `Param::Set()`
after every successful user change, and once at boot with `PARAM_LAST`. It:

- Re-selects modules when `Inverter`, `Vehicle`, `chargemodes`, `interface`, `Heater`,
  `BMS_Mode`, `DCdc_Type` or `GearLvr` change.
- Clears user CAN messages on both buses when a `...Can` bus assignment changes (which
  re-runs `SetCanFilters()`).
- Re-initialises CAN3 when `CAN3Speed` changes and TIM3 when PWM settings change.
- Forces `reversemotor` to 0 unless the inverter is `RearOutlander`.
- **Every time, whatever changed:** copies all throttle settings into `Throttle::`,
  refreshes the charge timer, re-runs the I/O matrix assignment and tells the
  preheater.

Because of the last point, a module that caches settings should read them in its own
tasks, or you add a hook here.

## Persistence

`save` (terminal, web UI or SDO) calls `parm_save()`, which writes every `PARAM_ENTRY`
value with its ID and flags into the **last flash page** and appends a CRC. The CAN map
is saved to its own pages (`CAN1_BLKNUM`, `CAN2_BLKNUM` in `hwdefs.h`). Interrupts are
disabled while writing.

At boot, `parm_load()` checks the CRC, then loads each stored value by ID. Stored IDs
that no longer exist are ignored; new parameters keep their defaults. That is what lets
a firmware update keep a configuration. Values are loaded without range checks.

`defaults` (terminal) reloads compiled defaults into RAM; it does not erase flash until
you `save`.

## Access paths

| Path | How | Notes |
|---|---|---|
| Web interface | ESP8266 module on the terminal UART | Uses the terminal commands below. |
| Terminal | USART3, 115200 baud by default | `set <name> <value>`, `get <name>`, `all`, `list`, `atr`, `json`, `save`, `load`, `defaults`, `stream`, `errors`, `can ...`, `reset`, `serial` |
| CAN SDO | Node ID 3 on CAN1: requests to `0x603`, replies on `0x583` | Index `0x2000` with sub-index = position in the list, or index `0x21hh` with sub-index `ll` for ID `0xhhll`. Data is the raw fixed-point value (value × 32). Also CAN map, error log and save/load/reset commands. |
| CAN map | `can rx <param> ...` | Writes a value directly from a received frame. See [CAN subsystem](can.md#canmap). |

## Adding a parameter

1. Pick the category and position (respecting the ordering rules above).
2. Add the `PARAM_ENTRY` (or `VALUE_ENTRY`) with the next free ID, and bump the
   "next id" comment.
3. If the unit is an option list, add a `#define` with the `"0=..., 1=..."` string next
   to the others.
4. Read it with `Param::GetInt(Param::yourParam)` in your module.
5. If changing it must trigger an action, add a `case` to `Param::Change()`.
6. Regenerate this site's reference pages with `scripts/gen_param_reference.py`.
