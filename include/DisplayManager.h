#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class DisplayManager {
private:
  Adafruit_SSD1306 _display;
  bool _initialized;

public:
  DisplayManager();

  bool begin();
  bool isReady() const;
  void showWelcome();
  void showConnectingWifi(const char* ssid);
  void showWifiConnected(const char* ip);
  void updateDashboard(int s1, int s2, int s3, int avg, const char* statusMsg, bool pumpActive, const char* ip = nullptr);
};
