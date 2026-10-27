#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "SoilSensor.h"
#include "PumpController.h"

class WebManager {
private:
  WebServer _server;
  SoilSensor& _soilSensor;
  PumpController& _pumpController;
  int& _threshold;
  bool& _autoWatering;
  const char* _ssid;
  const char* _password;
  const char* _hostname;
  bool _wifiConnected;

  void handleRoot();
  void handleGetStatus();
  void handlePostWater();
  void handlePostSettings();
  void handleNotFound();

public:
  WebManager(int port, 
             SoilSensor& soil, 
             PumpController& pump, 
             int& thresholdRef, 
             bool& autoWateringRef,
             const char* ssid, 
             const char* password, 
             const char* hostname);

  bool begin();
  void handleClient();
  bool isConnected() const;
  String getLocalIP() const;
  int getRSSI() const;
};
