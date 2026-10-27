#include "DisplayManager.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

DisplayManager::DisplayManager()
  : _display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1),
    _initialized(false) {}

bool DisplayManager::begin() {
  // SSD1306 standard I2C address is 0x3C
  if (_display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    _initialized = true;
    return true;
  }
  _initialized = false;
  return false;
}

bool DisplayManager::isReady() const {
  return _initialized;
}

void DisplayManager::showWelcome() {
  if (!_initialized) return;

  _display.clearDisplay();
  _display.setTextSize(1);
  _display.setTextColor(SSD1306_WHITE);
  _display.setCursor(16, 16);
  _display.println(F("SMART PLANT CARE"));
  _display.setCursor(20, 34);
  _display.println(F("ESP32 IoT System"));
  _display.display();
  delay(1200);
}

void DisplayManager::showConnectingWifi(const char* ssid) {
  if (!_initialized) return;

  _display.clearDisplay();
  _display.setTextSize(1);
  _display.setTextColor(SSD1306_WHITE);
  _display.setCursor(0, 10);
  _display.println(F("Connecting to Wi-Fi:"));
  _display.setCursor(0, 26);
  _display.println(ssid);
  _display.setCursor(0, 46);
  _display.println(F("Please wait..."));
  _display.display();
}

void DisplayManager::showWifiConnected(const char* ip) {
  if (!_initialized) return;

  _display.clearDisplay();
  _display.setTextSize(1);
  _display.setTextColor(SSD1306_WHITE);
  _display.setCursor(10, 10);
  _display.println(F("Wi-Fi Connected!"));
  _display.setCursor(0, 30);
  _display.println(F("IP Address:"));
  _display.setCursor(0, 46);
  _display.println(ip);
  _display.display();
  delay(1500);
}

void DisplayManager::updateDashboard(int percent, const char* statusMsg, bool pumpActive, const char* ip) {
  if (!_initialized) return;

  _display.clearDisplay();

  // Header Title
  _display.setTextSize(1);
  _display.setTextColor(SSD1306_WHITE);
  _display.setCursor(4, 2);
  _display.print(F("SMART PLANT CARE"));
  _display.drawLine(0, 12, 128, 12, SSD1306_WHITE);

  // Moisture Percent
  _display.setCursor(4, 18);
  _display.setTextSize(1);
  _display.print(F("Soil:"));

  _display.setTextSize(2);
  _display.setCursor(40, 15);
  _display.print(percent);
  _display.print(F("%"));

  // Progress Bar
  _display.drawRect(4, 33, 120, 8, SSD1306_WHITE);
  int barWidth = map(percent, 0, 100, 0, 116);
  if (barWidth > 0) {
    _display.fillRect(6, 35, barWidth, 4, SSD1306_WHITE);
  }

  // Status or IP Bar
  _display.setTextSize(1);
  _display.setCursor(4, 45);
  if (pumpActive) {
    _display.print(F(">> WATERING NOW! <<"));
  } else {
    _display.print(statusMsg);
  }

  // Bottom Line: IP Address if connected
  _display.setCursor(4, 55);
  if (ip != nullptr && strlen(ip) > 0) {
    _display.print(F("IP: "));
    _display.print(ip);
  } else {
    _display.print(F("Wi-Fi: Disconnected"));
  }

  _display.display();
}
