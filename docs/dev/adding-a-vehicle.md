# Adding a vehicle class

A vehicle class teaches the VCU about the car you are converting: where ignition and
start come from, how to drive the gauges, which gear the driver selected, and which
frames the car's remaining modules expect from the engine and gearbox computers that
were removed.

This walkthrough adds a class called `GM_GMT900` as option 9 of the `Vehicle`
parameter. The four edits to existing files and the two new files below were compiled
against V2.41A with `arm-none-eabi-gcc` 13.2 and build cleanly (flash grows by about
350 bytes for the skeleton). The [GMT900 worked example](gmt900-example.md) fills the
skeleton in for a real truck.

## Before you start

Decide, for your car:

| Question | Where it goes |
|---|---|
| What tells the VCU the ignition is on? A 12 V wire (T15 pin) or a CAN frame? | `Ready()` |
| What starts the car? The start input, a CAN start button, or "ignition on" is enough? | `Start()` |
| Where does the driver's gear selection come from? | `GetGear()`, or leave it to the fwd/rev inputs or a shifter class |
| Which gauges should show what? Tach, temperature, fuel/SOC? | `SetRevCounter()`, `SetTemperatureGauge()`, `SetFuelGauge()` |
| Which frames must be sent, at what rate, and what do the bytes mean? | `Task1Ms()`/`Task10Ms()`/`Task100Ms()`/`Task200Ms()` |
| Which frames from the car do you need to read? | `SetCanInterface()` registers them, `DecodeCAN()` reads them |

Capture a CAN log of the unmodified car first. Frame rates and byte meanings found on
forums are often for a different model year.

## Step 1: the header

Create `include/GM_GMT900.h`:

```cpp
#ifndef GM_GMT900_H
#define GM_GMT900_H

#include "digio.h"
#include "params.h"
#include "vehicle.h"
#include <stdint.h>

class GM_GMT900 : public Vehicle {
public:
  void SetCanInterface(CanHardware *c) override;
  void DecodeCAN(int id, uint32_t *data) override;
  void Task1Ms() override;
  void Task100Ms() override;
  void SetRevCounter(int speed) override { rpm = speed; }
  void SetTemperatureGauge(float temp) override { coolant = temp; }
  void SetFuelGauge(float level) override { fuel = level; }
  bool Ready() override { return DigIo::t15_digi.Get(); }

private:
  void Send0C9();
  void Send1F5();
  int rpm = 0;
  float coolant = 0;
  float fuel = 0;
  uint16_t tick = 0;
};

#endif
```

Notes:

- `Ready()`, `SetRevCounter()` and `SetTemperatureGauge()` are pure virtual in
  `Vehicle`; you must implement them.
- `SetRevCounter()` is called every 10 ms from the scheduler. Store the value and send it
  from your own task at the rate the car wants. Don't send CAN from the setter.
- Use `override` so the compiler catches a signature mismatch.
- Keep state in members. The object is static and lives for the whole run.

## Step 2: the implementation

Create `src/GM_GMT900.cpp`:

```cpp
#include "GM_GMT900.h"

void GM_GMT900::SetCanInterface(CanHardware *c) {
  can = c;
  // Register every ID you need to *receive* here, e.g.:
  // can->RegisterUserMessage(0x3E9);
}

void GM_GMT900::DecodeCAN(int id, uint32_t *data) {
  (void)id;
  (void)data;
}

void GM_GMT900::Task1Ms() {
  // 12.5 ms is not a multiple of the 10 ms task, so count 1 ms ticks.
  // 25 ticks = 25 ms (40 Hz); sending at ticks 0 and 12 gives ~80 Hz.
  tick++;
  if (tick % 25 == 0)
    Send1F5();
  if (tick % 25 == 0 || tick % 25 == 12)
    Send0C9();
  if (tick >= 1000)
    tick = 0;
}

void GM_GMT900::Task100Ms() {}

void GM_GMT900::Send0C9() {
  uint8_t bytes[8] = {0};
  uint16_t raw = rpm * 4; // 0.25 rpm/bit
  bytes[1] = raw >> 8;
  bytes[2] = raw & 0xFF;
  can->Send(0x0C9, bytes, 8);
}

void GM_GMT900::Send1F5() {
  uint8_t bytes[8] = {0};
  switch (Param::GetInt(Param::dir)) {
  case -1: bytes[3] = 2; break; // R
  case 1:  bytes[3] = 4; break; // D
  case 0:  bytes[3] = 3; break; // N
  default: bytes[3] = 1; break; // P
  }
  can->Send(0x1F5, bytes, 8);
}
```

The byte positions above are placeholders to show the mechanics; see the worked example
for what is known about the real frames.

## Step 3: give it a parameter value

In `include/param_prj.h`, three edits:

```diff
-  PARAM_ENTRY(CAT_SETUP, Vehicle, VEHMODES, 0, 8, 0, 6)                        \
+  PARAM_ENTRY(CAT_SETUP, Vehicle, VEHMODES, 0, 9, 0, 6)                        \
```

```diff
 #define VEHMODES                                                               \
   "0=BMW_E46, 1=BMW_E6x+, 2=Classic, 3=None, 5=BMW_E39, 6=VAG, 7=Subaru, "     \
-  "8=BMW_E31"
+  "8=BMW_E31, 9=GM_GMT900"
```

```diff
 enum vehicles {
   ...
   vSUBARU = 7,
-  vBMW_E31 = 8
+  vBMW_E31 = 8,
+  vGM_GMT900 = 9
 };
```

All three must agree. The `max` (9) is what lets the web UI and `set` accept the value;
the option string is what the drop-down shows; the enum is what the code switches on.
Do **not** change the parameter ID (6), or every saved configuration loses its vehicle
setting.

If you maintain a fork and upstream might add vehicle 9 later, pick a higher number
(for example 20) to avoid a clash.

## Step 4: instantiate and select it

In `src/stm32_vcu.cpp`:

```diff
 #include "Foccci.h"
+#include "GM_GMT900.h"
 #include "GS450H.h"
```

```diff
 static V_Classic classVehicle;
+static GM_GMT900 gmt900Vehicle;
```

```diff
 static void UpdateVehicle() {
   switch (Param::GetInt(Param::Vehicle)) {
   ...
   case vehicles::Classic:
     selectedVehicle = &classVehicle;
     break;
+  case vehicles::vGM_GMT900:
+    selectedVehicle = &gmt900Vehicle;
+    break;
   }
```

Includes are sorted alphabetically by clang-format, so `GM_GMT900.h` goes before
`GS450H.h`.

## Step 5: add it to the build

In the `Makefile`, append the object file to `OBJSL`:

```diff
-		   MGCoolantHeater.o NissLeafMng.o preheater.o ElconDCDC.o
+		   MGCoolantHeater.o NissLeafMng.o preheater.o ElconDCDC.o GM_GMT900.o
```

`vpath` finds `src/GM_GMT900.cpp` automatically.

## Step 6: build, format, test

```bash
make                       # must finish with the size report
pre-commit run --all-files # clang-format; CI fails without this
make Test && ./test/test_vcu
```

## Step 7: bench test

1. Flash, then in the web UI set `Vehicle` = `GM_GMT900` and `VehicleCan` to the bus
   wired to the car. `save`.
2. With a CAN adapter on that bus, confirm each frame's ID, rate and contents
   (`candump -td can0` on Linux shows the inter-frame time).
3. Check `cpuload`. A class that sends many frames from `Task1Ms()` shows up here.
4. Switch ignition on and off and watch `T15Stat` and `opmode`.
5. Only then connect to the car, with HV disconnected.

## Checklist

- [ ] `Ready()` returns false when the key is off, or the VCU can never leave Run.
- [ ] Every received ID is registered in `SetCanInterface()`.
- [ ] Nothing slow in `Task1Ms()` or `DecodeCAN()`.
- [ ] Frames you send do not collide with other modules on the same bus
      ([CAN IDs by module](../reference/can-ids.md)).
- [ ] If the class reads the gear selector, `GetGear()` returns true.
- [ ] `DashOff()` stops anything that should not run with the key off.
- [ ] `max`, option string and enum agree; parameter ID unchanged.
- [ ] Flash and RAM still fit (`make` prints sizes; see [Memory budget](build.md#memory-budget)).
