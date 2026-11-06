#pragma once
#include <Arduino.h>

// --- PIN DEFINITIONS ---
// Uses ADC1 pins exclusively because Wi-Fi is active (ADC2 is unavailable during Wi-Fi)
const int PIN_SOIL_1_ADC = 34;  // GPIO 34 (ADC1 channel 6 - input only)
const int PIN_SOIL_2_ADC = 33;  // GPIO 33 (ADC1 channel 5 - input/output capable)
const int PIN_SOIL_3_ADC = 32;  // GPIO 32 (ADC1 channel 4 - input/output capable)
const int PIN_RELAY_PUMP = 25;  // GPIO 25 (Digital Output)

// --- SENSOR CALIBRATION ---
// Raw ADC values (ESP32 ADC is 12-bit: 0 - 4095)
// Individual calibration per sensor (tune dry air = 0%, clean water = 100%)
const int SOIL_1_DRY_RAW = 3200;  // Sensor 1 (Biru) dry raw
const int SOIL_1_WET_RAW = 1400;  // Sensor 1 (Biru) wet raw

const int SOIL_2_DRY_RAW = 3200;  // Sensor 2 (Kuning) dry raw
const int SOIL_2_WET_RAW = 1400;  // Sensor 2 (Kuning) wet raw

const int SOIL_3_DRY_RAW = 3200;  // Sensor 3 (Merah) dry raw
const int SOIL_3_WET_RAW = 1400;  // Sensor 3 (Merah) wet raw

// --- BALANCED IRRIGATION & SAFETY LIMITS ---
// Default watering threshold percentage (below this value, plant is considered dry)
const int DEFAULT_DRY_THRESHOLD = 30;

// Maximum safe moisture limit. If ANY pot reaches this %, auto-watering is blocked
// to prevent waterlogging and root rot (akar reput).
const int SOIL_OVERWATER_LIMIT  = 85;

// --- TIMINGS (ms) ---
const unsigned long PUMP_RUN_DURATION_MS = 2500;   // Pump run duration: 2.5 seconds per watering
const unsigned long WATER_COOLDOWN_MS    = 15000;  // Cooldown time: 15 seconds before next watering

// --- WI-FI & WEB DASHBOARD ---
// Replace with your local Wi-Fi credentials
const char* WIFI_SSID     = "RumohTepiSejid_2.4GHz";
const char* WIFI_PASSWORD = "Abepi611_";
const char* MDNS_HOSTNAME = "plantcare";  // Access via http://plantcare.local
const int   WEB_SERVER_PORT = 80;
