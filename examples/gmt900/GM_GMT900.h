#ifndef GM_GMT900_H
#define GM_GMT900_H

/* GM GMT900 (2007-2014 Silverado/Sierra/Tahoe...) high-speed GMLAN integration.
 * Recreates the periodic ECM/TCM frames the rest of the truck expects after the
 * engine and 6L80 are removed. Frame contents are UNVERIFIED on a GMT900: fill
 * them in from a capture of a stock truck before enabling them.
 */

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
  void SetRevCounter(int speed) override { motorRpm = speed; }
  void SetTemperatureGauge(float temp) override { invTemp = temp; }
  void SetFuelGauge(float level) override { soc = level; }
  bool Ready() override { return DigIo::t15_digi.Get(); }
  void DashOff() override { rollingCtr = 0; }

private:
  struct Frame {
    uint16_t id;
    uint16_t period; // in 0.5 ms units, so 25 = 12.5 ms
    uint16_t phase;  // first send offset, 0.5 ms units, to stagger frames
    bool enabled;    // bring-up mask: enable one frame at a time
  };

  bool Build(uint16_t id, uint8_t bytes[8]);
  uint8_t TachRaw(uint16_t &raw);

  static Frame frames[];
  static const int numFrames;
  uint32_t nextDue[16] = {0};
  uint32_t now = 0; // 0.5 ms units
  bool started = false;
  int motorRpm = 0;
  float invTemp = 0;
  float soc = 0;
  uint8_t rollingCtr = 0;
  volatile uint16_t vehicleSpeedRaw = 0;
};

#endif
