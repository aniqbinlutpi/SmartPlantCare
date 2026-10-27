#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class DisplayManager {
private:
  Adafruit_SSD1306 _display;

public:
  DisplayManager();

  bool begin();
  void showWelcome();
  void updateDashboard(int percent, const char* statusMsg, bool pumpActive);
};
