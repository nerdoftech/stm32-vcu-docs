# Charging

## AC charging

Select your charger with `chargemodes` and its bus with `ChargerCan`. If you also use a
charge interface that handles the charge port pilot (BMW i3 LIM, FOCCCI, CPC), select it
with `interface`.

### When charging starts

Every 200 ms the VCU decides whether charging is wanted (`RunChg`):

| `Chgctrl` | Charging allowed |
|---|---|
| `Enable` (0) | Whenever plugged in, unless a previous charge ended normally (see lockout below). |
| `Disable` (1) | Never. If a proximity input is configured, plugging in still allows it. |
| `Timer` (2) | Starts at `Chg_Hrs`:`Chg_Min` and runs for `Chg_Dur` minutes. |

Then the charger driver decides, from `RunChg`, the charge interface's AC permission, and
its own plug/pilot state, whether to charge. If it says yes and the VCU is not in Run,
the VCU precharges and enters Charge.

### Proximity pilot

If an analogue input is set to `ProxPilot` (`GPA1Func` or `GPA2Func`), a reading at or
below `ppthresh` means a plug is in (`PlugDet` = 1). Unplugging always stops charging.
`DriveInhibit = Plug detect` keeps the car in neutral while plugged in.

### When charging ends

In Charge mode, charging stops and a **lockout** is set when:

- `udc` ≥ `Voltspnt` **and** `idc` ≤ `IdcTerm` (charged to target and current has tapered),
  or
- the BMS's allowed charge current (`BMS_ChargeLim`) is 0.

The lockout stops `Enable` mode from immediately restarting. It clears when you drive
(enter Run).

Other settings: `Pwrspnt` is the charge power request (W), passed to the charger.
`ChgAcVolt` and `ChgEff` let the LIM/CPC/FOCCCI convert power into an AC current.

### Setting the clock

The VCU has a simple real-time clock (day of week, hours, minutes) used by the charge and
preheat timers. It starts at zero on power-up. To set it, set `Chgctrl = Disable`, enter
`Set_Day`, `Set_Hour`, `Set_Min`, `Set_Sec`, then set `Chgctrl` back. The clock is only
copied in while `Chgctrl` is Disable. It keeps running while the board has 12 V.

## DC fast charging

Select `interface`: `i3LIM` (CCS via a BMW i3 charging module), `Foccci` (CCS via the
open-source FOCCCI board), `Chademo`, or `CPC`. Set `LimCan` to its bus.

- When the interface requests DC charging, the VCU precharges and closes the main
  contactor (`chgtyp` shows DCFC).
- `CCS_ILim` limits the charge current.
- `SOCFC` is the state of charge the VCU **reports** to the charger. It is a fixed value
  you set, not the measured SOC.
- The interface's DC contactor and charger negotiation are handled by the interface
  driver; watch `CCS_State`, `CCS_V`, `CCS_I` and `CCS_COND`.

With a LIM, FOCCCI or CPC, the VCU can also generate a fake control pilot (`CpSpoof` on a
PWM output) from the pilot current limit.

## External HV request

An input set to `HVRequest` (by default the HV request pin) brings HV up as if for DC
charging while it is on (`chgtyp` = 4). Useful for an external charger or service.

## Charging safety

- With a BMS selected, charging only runs while the BMS sends data and every cell is
  within `BMS_VminLimit`…`BMS_VmaxLimit` and `BMS_TminLimit`…`BMS_TmaxLimit`. If the BMS
  goes quiet for `BMS_Timeout` seconds, charging stops.
- `Voltspnt` is a pack-level backstop. Set it no higher than your BMS's limit.
