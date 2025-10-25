#include <Arduino.h>
/*
Name: Sarwat Fatima
Reg no.: 23-NTU-CS-1092
Class: BSCS-5-A
*/
const int buttonPin = 25;
const int ledPin = 4;
volatile bool ledState = LOW;
volatile unsigned long lastInterruptTime = 0;

void IRAM_ATTR handleButton() {
  unsigned long interruptTime = millis();
  // Debounce check (200ms threshold)
  if (interruptTime - lastInterruptTime > 200) {
    ledState = !ledState;  // Correct toggle operation
    digitalWrite(ledPin, ledState);
  }
  lastInterruptTime = interruptTime;
}

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(buttonPin), handleButton, FALLING);
  Serial.begin(115200);
}

void loop() {
  // Optional: Add non-critical tasks here
  Serial.print("Current LED State: ");
  Serial.println(ledState ? "ON" : "OFF");
  delay(1000);
}
