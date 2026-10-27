#pragma once
#include <Arduino.h>

// --- PIN DEFINITIONS ---
const int PIN_SOIL_ADC   = 34;  // GPIO 34 (ADC1 channel 6 - input only)
const int PIN_RELAY_PUMP = 25;  // GPIO 25 (Digital Output)

// --- SENSOR CALIBRATION ---
// Raw ADC values (ESP32 ADC is 12-bit: 0 - 4095)
const int SOIL_DRY_RAW = 3200;  // Analog value in dry air (0% moisture)
const int SOIL_WET_RAW = 1400;  // Analog value in water (100% moisture)

// Default watering threshold percentage (below this value, plant is considered dry)
const int DEFAULT_DRY_THRESHOLD = 30;

// --- TIMINGS (ms) ---
const unsigned long PUMP_RUN_DURATION_MS = 2500;   // Pump run duration: 2.5 seconds per watering
const unsigned long WATER_COOLDOWN_MS    = 15000;  // Cooldown time: 15 seconds before next watering

// --- WI-FI & WEB DASHBOARD ---
// Replace with your local Wi-Fi credentials
const char* WIFI_SSID     = "RumohTepiSejid_2.4GHz";
const char* WIFI_PASSWORD = "Abepi611_";
const char* MDNS_HOSTNAME = "plantcare";  // Access via http://plantcare.local
const int   WEB_SERVER_PORT = 80;
