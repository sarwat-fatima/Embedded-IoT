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

const int buttonPin = 4;  // Pin for the first button
const int resetButtonPin = 5; // Pin for the second button
const int led1Pin = 25;    // Pin for LED 1
const int led2Pin = 26;    // Pin for LED 2

int mode = 0; // Current mode
const int numModes = 5; // Number of modes

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(resetButtonPin, INPUT_PULLUP);
  pinMode(led1Pin, OUTPUT);
  pinMode(led2Pin, OUTPUT);

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
  if (digitalRead(buttonPin) == LOW) {
    delay(200); // Debounce delay
    mode = (mode + 1) % numModes; // Cycle through modes
    updateDisplay();
    updateLEDs();
    while (digitalRead(buttonPin) == LOW); // Wait for button release
  }



  // PWM fade for LED 1 and LED 2 in case 4
  if (mode == 4) {
    for (int brightness = 0; brightness <= 255; brightness++) {
      analogWrite(led1Pin, brightness); // Fade LED 1 up
      analogWrite(led2Pin, brightness); // Fade LED 2 up
      delay(10); // Adjust delay for fade speed
    }
    for (int brightness = 255; brightness >= 0; brightness--) {
      analogWrite(led1Pin, brightness); // Fade LED 1 down
      analogWrite(led2Pin, brightness); // Fade LED 2 down
      delay(10); // Adjust delay for fade speed
    }
  }
    // Reset mode to OFF and turn off LEDs when the second button is pressed
  if (digitalRead(resetButtonPin) == LOW) {
    delay(200); // Debounce delay
    mode = 0; // Reset to "Both OFF"
    updateDisplay();
    updateLEDs(); // Ensure LEDs are turned off
    while (digitalRead(resetButtonPin) == LOW); // Wait for button release
  }
}

void updateDisplay() {
  display.clearDisplay();
  display.setCursor(0, 0);
  
  switch (mode) {
    case 0:
      display.println("Mode: Both OFF");
      break;
    case 1:
      display.println("Mode: LED 1 ON");
      break;
    case 2:
      display.println("Mode: LED 2 ON");
      break;
    case 3:
      display.println("Mode: Both ON");
      break;
    case 4:
      display.println("Mode: PWM Fade");
      break;
  }
  
  display.display();
}

void updateLEDs() {
  switch (mode) {
    case 0:
      digitalWrite(led1Pin, LOW);
      digitalWrite(led2Pin, LOW);
      break;
    case 1:
      digitalWrite(led1Pin, HIGH);
      digitalWrite(led2Pin, LOW);
      break;
    case 2:
      digitalWrite(led1Pin, LOW);
      digitalWrite(led2Pin, HIGH);
      break;
    case 3:
      digitalWrite(led1Pin, HIGH);
      digitalWrite(led2Pin, HIGH);
      break;
    case 4:
      // PWM fading handled in loop
      break;
  }
}
