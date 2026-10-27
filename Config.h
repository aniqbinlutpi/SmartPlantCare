#pragma once
#include <Arduino.h>

// --- PIN DEFINITIONS ---
const int PIN_SOIL_ADC   = 34;  // GPIO 34 (ADC1)
const int PIN_RELAY_PUMP = 25;  // GPIO 25 (Digital Output)

// --- SENSOR CALIBRATION ---
const int SOIL_DRY_RAW = 3200;  // Nilai analog sensor di udara (0%)
const int SOIL_WET_RAW = 1400;  // Nilai analog sensor dalam air (100%)

// Ambang batas siraman (bawah nilai ini, siram)
const int DRY_THRESHOLD_PERCENT = 30;

// --- TIMINGS (ms) ---
const unsigned long PUMP_RUN_DURATION_MS = 2500;   // Pam menyiram selama 2.5s
const unsigned long WATER_COOLDOWN_MS    = 15000;  // Rehat 15s sebelum boleh siram lagi
