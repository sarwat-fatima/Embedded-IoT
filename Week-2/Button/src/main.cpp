#include <Arduino.h>
/*
Name: Sarwat Fatima
Reg no.: 23-NTU-Cs-1092
Class: BSCS-5-A
*/
#define BUTTON_PIN 4
 void setup() {
 pinMode(BUTTON_PIN, INPUT_PULLUP);
 Serial.begin(115200);
 }
 void loop() {
 if(digitalRead(BUTTON_PIN) == LOW)
 Serial.println("Button Pressed");
 else
 Serial.println("Button Released");
 }