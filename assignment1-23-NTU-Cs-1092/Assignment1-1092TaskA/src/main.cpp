/*
Name: Sarwat Fatima
Reg no.: 23-NTU-Cs-1092
Class: BSCS-5-A*/

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------- PIN ASSIGNMENTS ----------
const int LED_A_PIN = 23 ;
const int LED_B_PIN = 19;
const int LED_IND_PIN = 18;
const int BUTTON_MODE_PIN = 32;
const int BUTTON_RESET_PIN = 33;
const int BUZZER_PIN = 26;

// ---------- OLED ----------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
const uint8_t OLED_ADDR = 0x3C;

// ---------- MODE & TIMING ----------
int mode = 0; // 0..3
unsigned long lastButtonMillis = 0;
const unsigned long DEBOUNCE_MS = 200;

unsigned long lastBlinkMillis = 0;
const unsigned long BLINK_INTERVAL = 400;

int fadeValue = 0;
int fadeDirection = 1;
const int FADE_STEP = 4;
const int FADE_INTERVAL = 30;

// ---------- PWM (ledc) channels ----------
const int BUZZER_CHANNEL = 0;
const int LED_A_PWM_CHANNEL = 1;
const int LED_B_PWM_CHANNEL = 2;
const int LED_PWM_FREQ = 5000;
const int LED_PWM_RES = 8;

bool lastModeButtonState = HIGH;
bool lastResetButtonState = HIGH;

// ---------- Function Declarations ----------
void showModeOnOLED();
void updateOutputsForMode();
void beep(int ms = 120, int freq = 2000);

// ---------- Setup ----------
void setup() {
  Serial.begin(115200);
  delay(100);

  pinMode(LED_A_PIN, OUTPUT);
  pinMode(LED_B_PIN, OUTPUT);
  pinMode(LED_IND_PIN, OUTPUT);
  pinMode(BUTTON_MODE_PIN, INPUT_PULLUP);
  pinMode(BUTTON_RESET_PIN, INPUT_PULLUP);

  // Buzzer PWM
  ledcSetup(BUZZER_CHANNEL, 2000, 8);
  ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);

  // OLED setup
  Wire.begin(21, 22);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("SSD1306 allocation failed");
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  showModeOnOLED();
  updateOutputsForMode();
}

// ---------- OLED Display ----------
void showModeOnOLED() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.println("Mode:");
  display.setCursor(0, 28);
  switch (mode) {
    case 0: display.println("OFF"); break;
    case 1: display.println("ALT"); display.println("BLNK"); break;
    case 2: display.println("BOTH"); display.println("ON"); break;
    case 3: display.println("PWM"); display.println("FADE"); break;
    default: display.println("??"); break;
  }
  display.display();
}

// ---------- Buzzer Beep ----------
void beep(int ms, int freq) {
  ledcWriteTone(BUZZER_CHANNEL, freq);
  delay(ms);
  ledcWriteTone(BUZZER_CHANNEL, 0);
}

// ---------- Update Outputs ----------
void updateOutputsForMode() {
  // Reset everything
  digitalWrite(LED_A_PIN, LOW);
  digitalWrite(LED_B_PIN, LOW);
  digitalWrite(LED_IND_PIN, LOW);

  if (mode == 0) {
    // Both OFF
    // Detach PWM to allow digitalWrite
    ledcDetachPin(LED_A_PIN);
    ledcDetachPin(LED_B_PIN);
    digitalWrite(LED_A_PIN, LOW);
    digitalWrite(LED_B_PIN, LOW);
    digitalWrite(LED_IND_PIN, LOW);

  } else if (mode == 1) {
    // Alternate blink
    // Detach PWM to allow digitalWrite
    ledcDetachPin(LED_A_PIN);
    ledcDetachPin(LED_B_PIN);
    digitalWrite(LED_A_PIN, HIGH);
    digitalWrite(LED_B_PIN, LOW);
    digitalWrite(LED_IND_PIN, HIGH);
    lastBlinkMillis = millis();

  } else if (mode == 2) {
    // Both ON
    // Detach PWM to allow digitalWrite
    ledcDetachPin(LED_A_PIN);
    ledcDetachPin(LED_B_PIN);
    digitalWrite(LED_A_PIN, HIGH);
    digitalWrite(LED_B_PIN, HIGH);
    digitalWrite(LED_IND_PIN, LOW);

  } else if (mode == 3) {
    // PWM fade for both LEDs
    fadeValue = 0;
    fadeDirection = 1;
    ledcSetup(LED_A_PWM_CHANNEL, LED_PWM_FREQ, LED_PWM_RES);
    ledcSetup(LED_B_PWM_CHANNEL, LED_PWM_FREQ, LED_PWM_RES);
    ledcAttachPin(LED_A_PIN, LED_A_PWM_CHANNEL);
    ledcAttachPin(LED_B_PIN, LED_B_PWM_CHANNEL);
    digitalWrite(LED_IND_PIN, HIGH);
  }

  showModeOnOLED();
}

// ---------- Main Loop ----------
void loop() {
  unsigned long now = millis();

  bool modeBtn = digitalRead(BUTTON_MODE_PIN);
  bool resetBtn = digitalRead(BUTTON_RESET_PIN);

  // Mode button
  if (modeBtn == LOW && lastModeButtonState == HIGH && now - lastButtonMillis > DEBOUNCE_MS) {
    lastButtonMillis = now;
    mode = (mode + 1) % 4;
    updateOutputsForMode();
    beep(80, 2500);
  }
  lastModeButtonState = modeBtn;

  // Reset button
  if (resetBtn == LOW && lastResetButtonState == HIGH && now - lastButtonMillis > DEBOUNCE_MS) {
    lastButtonMillis = now;
    mode = 0;
    updateOutputsForMode();
    beep(160, 1500);
  }
  lastResetButtonState = resetBtn;

  // Mode Behaviors
  if (mode == 1) {
    // Alternate Blink
    if (now - lastBlinkMillis >= BLINK_INTERVAL) {
      lastBlinkMillis = now;
      digitalWrite(LED_A_PIN, !digitalRead(LED_A_PIN));
      digitalWrite(LED_B_PIN, !digitalRead(LED_B_PIN));
    }

  } else if (mode == 3) {
    // PWM Fade (both LEDs)
    if (now - lastBlinkMillis >= FADE_INTERVAL) {
      lastBlinkMillis = now;
      fadeValue += FADE_STEP * fadeDirection;
      if (fadeValue >= 255) { fadeValue = 255; fadeDirection = -1; }
      if (fadeValue <= 0) { fadeValue = 0; fadeDirection = 1; }
      ledcWrite(LED_A_PWM_CHANNEL, fadeValue);
      ledcWrite(LED_B_PWM_CHANNEL, fadeValue);
    }
  }

  delay(10);
}