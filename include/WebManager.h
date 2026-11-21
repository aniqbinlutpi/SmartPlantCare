#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include "SoilSensor.h"
#include "PumpController.h"

class WebManager {
private:
  WebServer _server;
  SoilSensor& _soil1;
  SoilSensor& _soil2;
  SoilSensor& _soil3;
  PumpController& _pumpController;
  int& _threshold;
  bool& _autoWatering;
  int _overwaterLimit;
  const char* _ssid;
  const char* _password;
  const char* _hostname;
  bool _wifiConnected;
  unsigned long _lastWifiRetry;
  unsigned long _lastRssiLog;
  unsigned long _lastRequestMs;   // last HTTP request served (link watchdog)
  unsigned long _lastHealMs;      // last forced reconnect
  Preferences _prefs;

  void handleRoot();
  void handleGetStatus();
  void handlePostWater();
  void handlePostSettings();
  void handleNotFound();
  void maintainWifi();
  void markActivity();
  void saveSettings();

public:
  WebManager(int port, 
             SoilSensor& soil1,
             SoilSensor& soil2,
             SoilSensor& soil3, 
             PumpController& pump, 
             int& thresholdRef, 
             bool& autoWateringRef,
             int overwaterLimit,
             const char* ssid, 
             const char* password, 
             const char* hostname);

  bool begin();
  void loadSettings(); // restore dashboard settings saved in flash
  void handleClient();
  bool isConnected() const;
  String getLocalIP() const;
  int getRSSI() const;
};
