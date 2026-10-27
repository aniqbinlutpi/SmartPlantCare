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
SoilSensor     soil(PIN_SOIL_ADC, SOIL_DRY_RAW, SOIL_WET_RAW);
PumpController pump(PIN_RELAY_PUMP, PUMP_RUN_DURATION_MS, WATER_COOLDOWN_MS);
DisplayManager displayMgr;
WebManager     webMgr(WEB_SERVER_PORT, soil, pump, dryThreshold, autoWateringEnabled, 
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

  // 3. Read current soil moisture
  int moisture = soil.getMoisturePercent();

  // 4. Automated Irrigation Logic
  if (autoWateringEnabled && soil.isDry(dryThreshold) && pump.canWater()) {
    Serial.println(F("[AUTO-WATER] Soil is dry! Activating pump..."));
    pump.requestWatering();
  }

  // 5. Update OLED Screen periodically
  if (millis() - lastScreenUpdate >= SCREEN_INTERVAL) {
    lastScreenUpdate = millis();

    const char* status = "HEALTHY";
    if (moisture < dryThreshold) {
      status = "DRY! NEED WATER";
    } else if (moisture > 80) {
      status = "VERY WET";
    }

    displayMgr.updateDashboard(
      moisture, 
      status, 
      pump.isWatering(), 
      webMgr.isConnected() ? webMgr.getLocalIP().c_str() : ""
    );
  }
}
