#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Pins
#define DHTPIN 23
#define DHTTYPE DHT11
#define BUTTON_PIN 5    // Button to GND, use INPUT_PULLUP

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // OLED Init
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found!");
    while (1);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("System Ready...");
  display.display();

  dht.begin();
}

void loop() {
  if (digitalRead(BUTTON_PIN) == LOW) {   // Button pressed
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Reading DHT...");
    display.display();
    delay(300); // debounce

    float h = dht.readHumidity();
    float t = dht.readTemperature(); // Celsius

    display.clearDisplay();

    if (isnan(t) || isnan(h)) {
      display.setCursor(0, 0);
      display.println("DHT Error!");
    } else {
      display.setCursor(0, 0);
      display.println("DHT22 Readings");
      display.setCursor(0, 20);
      display.print("Temp: ");
      display.print(t);
      display.println(" C");
      display.setCursor(0, 40);
      display.print("Hum:  ");
      display.print(h);
      display.println(" %");
    }

    display.display();
    delay(700);  // Wait before next press
  }
}
