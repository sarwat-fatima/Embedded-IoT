#include <Arduino.h>
/*
Name: Sarwat Fatima
Reg no.: 23-NTU-Cs-1092
Class: BSCS-5-A
*/
#define BUTTON_PIN 4
#define LED1_PIN 5
#define LED2_PIN 6

bool ledState = false;  // Toggle state tracker
int lastButtonState = HIGH;  // Previous button state

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  Serial.begin(115200);
}

void loop() {
  int currentButtonState = digitalRead(BUTTON_PIN);
  
  // Detect button press (falling edge)
  if (currentButtonState == LOW && lastButtonState == HIGH) {
    delay(50);  // Debounce delay
    ledState = !ledState;  // Toggle state
    
    // Control both LEDs based on state
    digitalWrite(LED1_PIN, ledState ? HIGH : LOW);
    digitalWrite(LED2_PIN, ledState ? LOW : HIGH);
    
    Serial.println(ledState ? "LED1 ON, LED2 OFF" : "LED1 OFF, LED2 ON");
  }
  
  lastButtonState = currentButtonState;
}
