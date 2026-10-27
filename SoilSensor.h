#pragma once
#include <Arduino.h>

class SoilSensor {
private:
  int _pin;
  int _dryVal;
  int _wetVal;

public:
  SoilSensor(int pin, int dryVal, int wetVal);

  int getRawValue();
  int getMoisturePercent();
  bool isDry(int thresholdPercent);
};
