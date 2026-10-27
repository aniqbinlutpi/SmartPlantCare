#include "DisplayManager.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

DisplayManager::DisplayManager()
  : _display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1) {}

bool DisplayManager::begin() {
  return _display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
}

void DisplayManager::showWelcome() {
  _display.clearDisplay();
  _display.setTextSize(1);
  _display.setTextColor(SSD1306_WHITE);
  _display.setCursor(16, 20);
  _display.println("SMART PLANT CARE");
  _display.setCursor(24, 38);
  _display.println("OOP Architecture");
  _display.display();
  delay(1800);
}

void DisplayManager::updateDashboard(int percent, const char* statusMsg, bool pumpActive) {
  _display.clearDisplay();

  // Header Title
  _display.setTextSize(1);
  _display.setTextColor(SSD1306_WHITE);
  _display.setCursor(16, 2);
  _display.print("SMART PLANT CARE");
  _display.drawLine(0, 12, 128, 12, SSD1306_WHITE);

  // Moisture Percent
  _display.setCursor(4, 20);
  _display.setTextSize(1);
  _display.print("Soil:");

  _display.setTextSize(2);
  _display.setCursor(40, 18);
  _display.print(percent);
  _display.print("%");

  // Progress Bar
  _display.drawRect(4, 38, 120, 10, SSD1306_WHITE);
  int barWidth = map(percent, 0, 100, 0, 116);
  _display.fillRect(6, 40, barWidth, 6, SSD1306_WHITE);

  // Status Bar
  _display.setTextSize(1);
  _display.setCursor(4, 52);
  if (pumpActive) {
    _display.print(">> WATERING NOW! <<");
  } else {
    _display.print("Status: ");
    _display.print(statusMsg);
  }

  _display.display();
}
