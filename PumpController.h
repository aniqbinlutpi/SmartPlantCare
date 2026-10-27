#pragma once
#include <Arduino.h>

class PumpController {
private:
  int _relayPin;
  unsigned long _runDuration;
  unsigned long _cooldownTime;
  unsigned long _lastWateredTime;
  unsigned long _pumpStartTime;
  bool _isWatering;

public:
  PumpController(int relayPin, unsigned long runDuration, unsigned long cooldownTime);

  void begin();
  bool requestWatering();
  void update(); // Non-blocking: semak sama ada masa siraman tamat
  bool isWatering() const;
  bool canWater() const;
};
