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

void DisplayManager::updateDashboard(int s1, int s2, int s3, int avg, const char* statusMsg, bool pumpActive, const char* ip) {
  if (!_initialized) return;

  _display.clearDisplay();

  // Header Title
  _display.setTextSize(1);
  _display.setTextColor(SSD1306_WHITE);
  _display.setCursor(10, 2);
  _display.print(F("SMART PLANT CARE (3S)"));
  _display.drawLine(0, 11, 128, 11, SSD1306_WHITE);

  // Sensor 1 and Sensor 2
  _display.setCursor(2, 14);
  _display.print(F("S1:"));
  _display.print(s1);
  _display.print(F("%"));

  _display.setCursor(68, 14);
  _display.print(F("S2:"));
  _display.print(s2);
  _display.print(F("%"));

  // Sensor 3 and Average
  _display.setCursor(2, 25);
  _display.print(F("S3:"));
  _display.print(s3);
  _display.print(F("%"));

  _display.setCursor(68, 25);
  _display.print(F("AVG:"));
  _display.print(avg);
  _display.print(F("%"));

  // Progress Bar for Average Moisture
  _display.drawRect(2, 36, 124, 7, SSD1306_WHITE);
  int barWidth = map(constrain(avg, 0, 100), 0, 100, 0, 120);
  if (barWidth > 0) {
    _display.fillRect(4, 38, barWidth, 3, SSD1306_WHITE);
  }

  // Pump / System Status Line
  _display.setTextSize(1);
  _display.setCursor(2, 46);
  if (pumpActive) {
    _display.print(F(">> WATERING NOW! <<"));
  } else {
    _display.print(statusMsg);
  }

  // Bottom Line: IP Address or Offline
  _display.setCursor(2, 56);
  if (ip != nullptr && strlen(ip) > 0) {
    _display.print(F("IP:"));
    _display.print(ip);
  } else {
    _display.print(F("Wi-Fi: Disconnected"));
  }

  _display.display();
}
