#include "GM_GMT900.h"
#include "my_math.h"

// ---- tuning constants -------------------------------------------------
static const int TACH_IDLE_RPM = 700;   // shown while READY and stopped
static const int TACH_DIVIDER = 2;      // LDU rpm / 2 fits a ~6k tach
static const int COOLANT_NORMAL_C = 90; // needle position for "normal"

// ID, period (0.5 ms), phase (0.5 ms), enabled
// Rates come from a non-GMT900 GM capture; verify on the truck.
GM_GMT900::Frame GM_GMT900::frames[] = {
    {0x0C9, 25, 0, true},    // ECM: rpm, run status, brake   12.5 ms
    {0x0F9, 25, 3, false},   // ECM/TCM: undecoded           12.5 ms
    {0x1ED, 25, 6, false},   // ECM: undecoded               12.5 ms
    {0x1EF, 25, 9, false},   // ECM: rpm (alternate)         12.5 ms
    {0x1A1, 50, 12, false},  // ECM: accelerator             25 ms
    {0x1C3, 50, 15, false},  // ECM: torque / accelerator    25 ms
    {0x1F5, 50, 18, true},   // TCM: PRNDL                   25 ms
    {0x2C3, 100, 21, false}, // ECM: undecoded               50 ms
    {0x3C1, 200, 24, false}, // ECM: undecoded               100 ms
    {0x3D1, 200, 27, false}, // ECM: undecoded               100 ms
    {0x3F9, 500, 30, false}, // ECM/TCM: undecoded           250 ms
    {0x3FB, 500, 33, false}, // ECM/TCM: undecoded           250 ms
    {0x4C1, 1000, 36, true}, // ECM: coolant, IAT, OAT       500 ms
    {0x4D1, 1000, 39, true}, // ECM: oil temp, fuel level?   500 ms
};
const int GM_GMT900::numFrames = sizeof(frames) / sizeof(frames[0]);

void GM_GMT900::SetCanInterface(CanHardware *c) {
  can = c;
  can->RegisterUserMessage(0x3E9); // EBCM wheel speeds (placeholder decode)
}

void GM_GMT900::DecodeCAN(int id, uint32_t *data) {
  uint8_t *bytes = (uint8_t *)data;
  if (id == 0x3E9) {
    // UNVERIFIED layout: one 16-bit store so tasks never see half an update
    vehicleSpeedRaw = (bytes[0] << 8) | bytes[1];
  }
}

void GM_GMT900::Task1Ms() {
  if (!Ready()) { // stay silent with the key off so the bus can sleep
    started = false;
    return;
  }
  if (!started) {
    now = 0;
    for (int i = 0; i < numFrames; i++)
      nextDue[i] = frames[i].phase;
    started = true;
  }

  for (int i = 0; i < numFrames; i++) {
    if (now < nextDue[i])
      continue;
    nextDue[i] += frames[i].period;
    uint8_t bytes[8] = {0};
    if (frames[i].enabled && Build(frames[i].id, bytes))
      can->Send(frames[i].id, bytes, 8);
  }
  now += 2; // 1 ms
}

void GM_GMT900::Task100Ms() { rollingCtr = (rollingCtr + 1) & 0x3; }

uint8_t GM_GMT900::TachRaw(uint16_t &raw) {
  int rpm = ABS(motorRpm) / TACH_DIVIDER;
  if (Param::GetInt(Param::opmode) == MOD_RUN)
    rpm = MAX(rpm, TACH_IDLE_RPM);
  raw = rpm * 4; // 0.25 rpm/bit
  return Param::GetInt(Param::opmode) == MOD_RUN ? 1 : 0;
}

// Fill bytes[] for one frame. Return false to send nothing (contents unknown).
// Byte indexes are 0-based. Forum notes usually count from 1.
bool GM_GMT900::Build(uint16_t id, uint8_t bytes[8]) {
  switch (id) {
  case 0x0C9: {
    uint16_t raw;
    uint8_t running = TachRaw(raw);
    bytes[0] = running ? 0x80 : 0x00; // run status bit: UNVERIFIED
    bytes[1] = raw >> 8;              // "bytes 2-3" in forum notes
    bytes[2] = raw & 0xFF;
    if (Param::GetBool(Param::din_brake))
      bytes[5] |= 0x01; // "brake bit in the 6th byte": bit UNVERIFIED
    return true;
  }
  case 0x1F5: {
    uint8_t prndl;
    if (Param::GetInt(Param::opmode) != MOD_RUN)
      prndl = 1; // Park
    else {
      switch (Param::GetInt(Param::dir)) {
      case 1:
        prndl = 4;
        break; // Drive
      case -1:
        prndl = 2;
        break; // Reverse
      case 0:
        prndl = 3;
        break; // Neutral
      default:
        prndl = 1;
        break; // Park
      }
    }
    bytes[3] = prndl; // "byte 4" in forum notes
    return true;
  }
  case 0x4C1: {
    // Hold the needle at normal until the inverter is genuinely hot.
    int t = MAX(COOLANT_NORMAL_C, (int)invTemp);
    bytes[1] = t + 40;  // "bytes 2-4 = A-40": coolant
    bytes[2] = 20 + 40; // intake air, fixed
    bytes[3] = 20 + 40; // outside air, fixed (better: leave to BCM)
    return true;
  }
  case 0x4D1:
    bytes[1] = COOLANT_NORMAL_C + 40; // oil temp; fuel byte UNKNOWN
    return true;
  default:
    return false; // paste captured bytes here before enabling
  }
}
