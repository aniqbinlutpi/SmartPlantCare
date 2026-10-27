#include "PumpController.h"

PumpController::PumpController(int relayPin, unsigned long runDuration, unsigned long cooldownTime)
  : _relayPin(relayPin),
    _runDuration(runDuration),
    _cooldownTime(cooldownTime),
    _lastWateredTime(0),
    _pumpStartTime(0),
    _isWatering(false) {}

void PumpController::begin() {
  pinMode(_relayPin, OUTPUT);
  digitalWrite(_relayPin, LOW); // Relay OFF pada permulaan
}

bool PumpController::canWater() const {
  if (_isWatering) return false;
  return (millis() - _lastWateredTime >= _cooldownTime);
}

bool PumpController::requestWatering() {
  if (!canWater()) return false;

  _isWatering = true;
  _pumpStartTime = millis();
  digitalWrite(_relayPin, HIGH); // Hidupkan pam
  return true;
}

void PumpController::update() {
  if (_isWatering) {
    // Semak sama ada masa siram sudah cukup
    if (millis() - _pumpStartTime >= _runDuration) {
      digitalWrite(_relayPin, LOW); // Matikan pam
      _isWatering = false;
      _lastWateredTime = millis();  // Mula timer cooldown
    }
  }
}

bool PumpController::isWatering() const {
  return _isWatering;
}
