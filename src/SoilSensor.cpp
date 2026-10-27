#include "SoilSensor.h"

SoilSensor::SoilSensor(int pin, int dryVal, int wetVal)
  : _pin(pin), _dryVal(dryVal), _wetVal(wetVal) {}

int SoilSensor::getRawValue() {
  return analogRead(_pin);
}

int SoilSensor::getMoisturePercent() {
  int raw = getRawValue();
  // Untuk capacitive sensor: nilai tinggi = kering, nilai rendah = basah
  int percent = map(raw, _dryVal, _wetVal, 0, 100);
  return constrain(percent, 0, 100);
}

bool SoilSensor::isDry(int thresholdPercent) {
  return getMoisturePercent() < thresholdPercent;
}
