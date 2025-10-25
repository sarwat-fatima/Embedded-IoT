#include <Arduino.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---- OLED setup ----
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  Wire.begin(21, 22); // ESP32 default I2C pins (SDA=21, SCL=22)

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    // If initialization fails
    for (;;);
  }

  display.clearDisplay();

  // ---- Display Name and ID ----
  display.setTextSize(1);              // Normal text size
  display.setTextColor(SSD1306_WHITE); // White color
  display.setCursor(25, 20);           // X, Y position
  display.println("Sarwat Fatima");
  display.setCursor(10, 35);
  display.println("23-NTU-CS-1092");

  // Draw rectangle box around text
  display.drawRect(5, 15, 118, 40, SSD1306_WHITE);
  
  display.display();
}

void loop() {
  // Nothing to loop — static display
}