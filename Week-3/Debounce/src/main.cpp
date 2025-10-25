#include <Arduino.h>

#define LED_PIN        4
#define BUTTON_PIN     27
#define DEBOUNCE_MS    50
#define PRESCALER      80      // 80MHz clock / 80 = 1MHz timer

// Timer configuration
hw_timer_t* debounceTimer = nullptr;
volatile bool debounceActive = false;

void ARDUINO_ISR_ATTR onDebounceTimer() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  }
  timerAlarmDisable(debounceTimer);
  debounceActive = false;
}

void ARDUINO_ISR_ATTR onButtonISR() {
  if (!debounceActive) {
    debounceActive = true;
    timerWrite(debounceTimer, 0);  // Reset timer counter
    timerAlarmEnable(debounceTimer);
  }
}

void setup() {
  // Initialize GPIO
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  // Configure interrupt
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), onButtonISR, FALLING);

  // Timer initialization
  debounceTimer = timerBegin(0, PRESCALER, true);  // Timer 0, 1MHz frequency
  timerAttachInterrupt(debounceTimer, &onDebounceTimer, true);
  timerAlarmWrite(debounceTimer, DEBOUNCE_MS * 1000, false);
}

void loop() {

  
}
