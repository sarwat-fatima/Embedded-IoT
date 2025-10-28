#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/*Name: Sarwat Fatima regno: 23-NTU-Cs-1092 , BSCS-5-A*/

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
//#define OLED_RESET -1

// Define custom I2C pins
#define OLED_SDA 21 // Change to your SDA pin
#define OLED_SCL 22 // Change to your SCL pin

// Use the correct I2C address for your OLED display
#define OLED_I2C_ADDRESS 0x3C 

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const int buttonPin = 4;  // Pin for the button
const int led1Pin = 25;    // Pin for LED 1
const int buzzerPin = 18;  // Pin for the buzzer

bool ledState = false; // State of the LED
unsigned long buttonPressStart = 0; // Time when button was pressed
bool buttonPressed = false; // Flag to check if the button is pressed
void updateDisplay(String message) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(message);
  display.display();
}

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(led1Pin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  // Initialize I2C with custom SDA and SCL pins
  Wire.begin(OLED_SDA, OLED_SCL);
  
  // Initialize the display with the correct I2C address
  display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 10);
  display.println("Hello IoT!");
  display.display();
}

void loop() {
  // Check button state
  if (digitalRead(buttonPin) == LOW) {
    if (!buttonPressed) {
      // Button just pressed
      buttonPressed = true;
      buttonPressStart = millis(); // Record the time when button was pressed
    }
  } else {
    // Button released
    if (buttonPressed) {
      buttonPressed = false;
      unsigned long pressDuration = millis() - buttonPressStart;

      if (pressDuration < 1500) {
        // Short press: toggle LED
        ledState = !ledState;
        digitalWrite(led1Pin, ledState ? HIGH : LOW);
        updateDisplay("LED: " + String(ledState ? "ON" : "OFF"));
      } else {
        // Long press: play buzzer tone
        tone(buzzerPin, 1000); // Play tone at 1000 Hz
        updateDisplay("Buzzer: ON");
        delay(1000); // Duration of the tone
        noTone(buzzerPin); // Stop the tone
        updateDisplay("Buzzer: OFF");
      }
    }
  }
}

