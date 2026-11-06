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
SoilSensor     soil1(PIN_SOIL_1_ADC, SOIL_1_DRY_RAW, SOIL_1_WET_RAW);
SoilSensor     soil2(PIN_SOIL_2_ADC, SOIL_2_DRY_RAW, SOIL_2_WET_RAW);
SoilSensor     soil3(PIN_SOIL_3_ADC, SOIL_3_DRY_RAW, SOIL_3_WET_RAW);

PumpController pump(PIN_RELAY_PUMP, PUMP_RUN_DURATION_MS, WATER_COOLDOWN_MS);
DisplayManager displayMgr;
WebManager     webMgr(WEB_SERVER_PORT, soil1, soil2, soil3, pump, dryThreshold, autoWateringEnabled, 
                      SOIL_OVERWATER_LIMIT, WIFI_SSID, WIFI_PASSWORD, MDNS_HOSTNAME);

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

  // 4. Smart Balanced Irrigation Logic (1 Shared Pump -> 3 Pots)
  // Rule A (Anti-Rot Safeguard): If ANY pot is already saturated (>= SOIL_OVERWATER_LIMIT),
  // auto-watering MUST be locked to protect roots from drowning/root rot.
  bool isOverwaterLocked = (m1 >= SOIL_OVERWATER_LIMIT || m2 >= SOIL_OVERWATER_LIMIT || m3 >= SOIL_OVERWATER_LIMIT);

  // Rule B (Dryness Detection):
  bool isAnyDry = (soil1.isDry(dryThreshold) || soil2.isDry(dryThreshold) || soil3.isDry(dryThreshold));
  bool isAvgDry = (avgMoisture < dryThreshold);

  // Rule C (Balanced Trigger):
  // Siram jika purata tanah kering, ATAU jika ada bahagian kering tetapi purata belum basah (< 60%).
  // Wajib lulus safeguard (tidak terkunci overwater).
  bool needsWater = !isOverwaterLocked && (isAvgDry || (isAnyDry && avgMoisture < 60));

  if (autoWateringEnabled && needsWater && pump.canWater()) {
    Serial.printf("[AUTO-WATER] Balanced watering burst: S1:%d%% S2:%d%% S3:%d%% | AVG:%d%% (Thresh:%d%%)\n",
                  m1, m2, m3, avgMoisture, dryThreshold);
    pump.requestWatering();
  } else if (autoWateringEnabled && isOverwaterLocked && isAnyDry) {
    // Periodic alert in serial monitor when a pot is held from overwatering
    static unsigned long lastWarnTime = 0;
    if (millis() - lastWarnTime >= 10000) {
      lastWarnTime = millis();
      Serial.printf("[SAFEGUARD] Auto-watering paused: S1:%d%% S2:%d%% S3:%d%%. Prevented drowning pot with >= %d%% moisture.\n",
                    m1, m2, m3, SOIL_OVERWATER_LIMIT);
    }
  }

  // 5. Update OLED Screen periodically
  if (millis() - lastScreenUpdate >= SCREEN_INTERVAL) {
    lastScreenUpdate = millis();

    const char* status = "HEALTHY";
    if (isOverwaterLocked) {
      status = "HOLD: SOIL OVERWET";
    } else if (needsWater) {
      status = "DRY! NEED WATER";
    } else if (avgMoisture > 80) {
      status = "ALL VERY WET";
    }

    displayMgr.updateDashboard(
      m1, m2, m3, avgMoisture,
      status, 
      pump.isWatering(), 
      webMgr.isConnected() ? webMgr.getLocalIP().c_str() : ""
    );
  }
}
