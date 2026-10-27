#include "Config.h"
#include "SoilSensor.h"
#include "PumpController.h"
#include "DisplayManager.h"

// Inisialisasi Objek (OOP)
SoilSensor     soil(PIN_SOIL_ADC, SOIL_DRY_RAW, SOIL_WET_RAW);
PumpController pump(PIN_RELAY_PUMP, PUMP_RUN_DURATION_MS, WATER_COOLDOWN_MS);
DisplayManager displayMgr;

unsigned long lastScreenUpdate = 0;
const unsigned long SCREEN_INTERVAL = 500; // Kemas kini skrin setiap 500ms

void setup() {
  Serial.begin(115200);

  // Inisialisasi modul
  pump.begin();

  if (!displayMgr.begin()) {
    Serial.println(F("Gagal memulakan OLED!"));
    while (1);
  }

  displayMgr.showWelcome();
}

void loop() {
  // 1. Sentiasa kemas kini pam (non-blocking timer keselamatan)
  pump.update();

  // 2. Baca nilai kelembapan tanah
  int moisture = soil.getMoisturePercent();

  // 3. Logik Automatik: Jika tanah kering & cooldown selesai, pam mula menyiram
  if (soil.isDry(DRY_THRESHOLD_PERCENT) && pump.canWater()) {
    Serial.println("Tanah kering! Mengaktifkan pam air...");
    pump.requestWatering();
  }

  // 4. Kemas kini paparan skrin OLED
  if (millis() - lastScreenUpdate >= SCREEN_INTERVAL) {
    lastScreenUpdate = millis();

    const char* status = "HEALTHY";
    if (moisture < DRY_THRESHOLD_PERCENT) {
      status = "DRY! NEED WATER";
    } else if (moisture > 80) {
      status = "VERY WET";
    }

    displayMgr.updateDashboard(moisture, status, pump.isWatering());

    // Debugging di Serial Monitor
    Serial.print("Raw ADC: ");
    Serial.print(soil.getRawValue());
    Serial.print(" | Soil: ");
    Serial.print(moisture);
    Serial.print("% | Pump: ");
    Serial.println(pump.isWatering() ? "ON" : "OFF");
  }
}
