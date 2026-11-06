#include <Arduino.h>
#include "Config.h"
#include "SoilSensor.h"
#include "PumpController.h"
#include "DisplayManager.h"
#include "WebManager.h"

// Runtime configurable settings (can be adjusted live via the Web Dashboard)
int dryThreshold = DEFAULT_DRY_THRESHOLD;
bool autoWateringEnabled = true;

// Object Instantiations (Clean OOP Architecture)
SoilSensor     soil1(PIN_SOIL_1_ADC, SOIL_DRY_RAW, SOIL_WET_RAW);
SoilSensor     soil2(PIN_SOIL_2_ADC, SOIL_DRY_RAW, SOIL_WET_RAW);
SoilSensor     soil3(PIN_SOIL_3_ADC, SOIL_DRY_RAW, SOIL_WET_RAW);

PumpController pump(PIN_RELAY_PUMP, PUMP_RUN_DURATION_MS, WATER_COOLDOWN_MS);
DisplayManager displayMgr;
WebManager     webMgr(WEB_SERVER_PORT, soil1, soil2, soil3, pump, dryThreshold, autoWateringEnabled, 
                      WIFI_SSID, WIFI_PASSWORD, MDNS_HOSTNAME);

unsigned long lastScreenUpdate = 0;
const unsigned long SCREEN_INTERVAL = 500; // Update display every 500ms

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println(F("\n========================================"));
  Serial.println(F("   🌱 ESP32 SMART PLANT CARE SYSTEM    "));
  Serial.println(F("========================================"));

  // 1. Initialize Relay / Water Pump
  pump.begin();
  Serial.println(F("[PUMP] Controller initialized."));

  // 2. Initialize OLED Display (Non-blocking: skips gracefully if OLED is absent)
  if (displayMgr.begin()) {
    Serial.println(F("[OLED] Display detected and initialized."));
    displayMgr.showWelcome();
    displayMgr.showConnectingWifi(WIFI_SSID);
  } else {
    Serial.println(F("[OLED] Warning: Display not found. Continuing in headless mode."));
  }

  // 3. Connect to Wi-Fi and start Web Dashboard
  bool wifiOk = webMgr.begin();
  if (wifiOk && displayMgr.isReady()) {
    displayMgr.showWifiConnected(webMgr.getLocalIP().c_str());
  }

  Serial.println(F("\nSystem ready! Access live dashboard at:"));
  Serial.print(F(" - http://"));
  Serial.println(webMgr.getLocalIP());
  Serial.print(F(" - http://"));
  Serial.print(MDNS_HOSTNAME);
  Serial.println(F(".local (mDNS)\n"));
}

void loop() {
  // 1. Update pump safety timer (non-blocking shut-off)
  pump.update();

  // 2. Handle HTTP clients for live web dashboard
  webMgr.handleClient();

  // 3. Read current soil moistures & compute average
  int m1 = soil1.getMoisturePercent();
  int m2 = soil2.getMoisturePercent();
  int m3 = soil3.getMoisturePercent();
  int avgMoisture = (m1 + m2 + m3) / 3;

  // 4. Automated Irrigation Logic
  // Check if any zone is dry OR if average moisture is below threshold
  bool isAnyDry = soil1.isDry(dryThreshold) || soil2.isDry(dryThreshold) || soil3.isDry(dryThreshold);
  bool needsWater = (avgMoisture < dryThreshold) || isAnyDry;

  if (autoWateringEnabled && needsWater && pump.canWater()) {
    Serial.printf("[AUTO-WATER] S1:%d%% S2:%d%% S3:%d%% | AVG:%d%% (Thresh:%d%%). Activating pump...\n",
                  m1, m2, m3, avgMoisture, dryThreshold);
    pump.requestWatering();
  }

  // 5. Update OLED Screen periodically
  if (millis() - lastScreenUpdate >= SCREEN_INTERVAL) {
    lastScreenUpdate = millis();

    const char* status = "HEALTHY";
    if (needsWater) {
      status = "DRY! NEED WATER";
    } else if (avgMoisture > 80) {
      status = "VERY WET";
    }

    displayMgr.updateDashboard(
      m1, m2, m3, avgMoisture,
      status, 
      pump.isWatering(), 
      webMgr.isConnected() ? webMgr.getLocalIP().c_str() : ""
    );
  }
}
