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
  digitalWrite(_relayPin, LOW); // Relay OFF at startup (assuming Active-HIGH relay)
}

bool PumpController::canWater() const {
  if (_isWatering) return false;
  if (_lastWateredTime == 0) return true; // Never watered yet
  return (millis() - _lastWateredTime >= _cooldownTime);
}

bool PumpController::requestWatering() {
  if (!canWater()) return false;

  _isWatering = true;
  _pumpStartTime = millis();
  digitalWrite(_relayPin, HIGH); // Activate pump
  return true;
}

void PumpController::forceStop() {
  if (_isWatering) {
    digitalWrite(_relayPin, LOW);
    _isWatering = false;
    _lastWateredTime = millis();
  }
}

void PumpController::update() {
  if (_isWatering) {
    // Check if run duration has elapsed
    if (millis() - _pumpStartTime >= _runDuration) {
      digitalWrite(_relayPin, LOW); // Turn off pump
      _isWatering = false;
      _lastWateredTime = millis();  // Start cooldown timer
    }
  }
}

bool PumpController::isWatering() const {
  return _isWatering;
}

unsigned long PumpController::getRemainingCooldownMs() const {
  if (_isWatering) return _runDuration;
  if (_lastWateredTime == 0) return 0;
  
  unsigned long elapsed = millis() - _lastWateredTime;
  if (elapsed >= _cooldownTime) return 0;
  return _cooldownTime - elapsed;
}

unsigned long PumpController::getLastWateredTime() const {
  return _lastWateredTime;
}

unsigned long PumpController::getRunDuration() const {
  return _runDuration;
}
