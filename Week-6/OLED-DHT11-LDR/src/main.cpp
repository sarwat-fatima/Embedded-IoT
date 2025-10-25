#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
/* 
Name: Sarwat Fatima
Reg no. : 23-NTU-CS-1092
SEction : BSCS-5-A
*/

// --- Pin configuration ---
#define LDR_PIN    34   // ADC pin for LDR
#define DHTPIN     14   // DHT data pin
// If your DHT is DHT11 change DHTTYPE to DHT11
#define DHTTYPE    DHT22

#define SDA_PIN    21   // I2C SDA
#define SCL_PIN    22   // I2C SCL

// --- OLED setup ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// --- DHT sensor setup ---
DHT dht(DHTPIN, DHTTYPE);

// Update interval (ms)
const unsigned long UPDATE_INTERVAL_MS = 2000;
unsigned long lastUpdate = 0;

void setup() {
  Serial.begin(115200);
  delay(100);

  // Initialize I2C on custom pins
  Wire.begin(SDA_PIN, SCL_PIN);

  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    for (;;); // halt if display init fails
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Initializing...");
  display.display();

  // Initialize DHT sensor
  dht.begin();

  // small warm-up delay
  delay(1000);
}

void loop() {
  unsigned long now = millis();
  if (now - lastUpdate < UPDATE_INTERVAL_MS) return;
  lastUpdate = now;

  // Read LDR ADC
  int adcValue = analogRead(LDR_PIN);
  float voltage = (adcValue / 4095.0f) * 3.3f; // 12-bit ADC assumed

  // Read DHT
  float temperature = dht.readTemperature(); // Celsius
  float humidity = dht.readHumidity();

  // Check read failure
  bool dhtValid = true;
  if (isnan(temperature) || isnan(humidity)) {
    dhtValid = false;
    Serial.println("Error reading DHT sensor!");
  }

  // Serial output
  Serial.printf("LDR ADC: %d  |  Voltage: %.2f V", adcValue, voltage);
  if (dhtValid) {
    Serial.printf("  |  Temp: %.2f C  |  Humidity: %.2f %%\n", temperature, humidity);
  } else {
    Serial.println();
  }

  // Display output
  display.clearDisplay();

  // Top: LDR
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("LDR ADC: ");
  display.println(adcValue);

  display.setCursor(0, 10);
  display.print("Voltage: ");
  display.print(voltage, 2);
  display.println(" V");

  // Divider line (thin)
  display.drawFastHLine(0, 24, SCREEN_WIDTH, SSD1306_WHITE);

  // DHT values
  display.setTextSize(1);
  display.setCursor(0, 28);
  if (dhtValid) {
    display.print("Temp: ");
    display.setTextSize(2);
    display.print(temperature, 1);
    display.setTextSize(1);
    display.println(" C");

    display.setCursor(0, 48);
    display.print("Humidity: ");
    display.setTextSize(2);
    display.print(humidity, 1);
    display.setTextSize(1);
    display.println(" %");
  } else {
    display.println("DHT read error");
  }

  display.display();
}