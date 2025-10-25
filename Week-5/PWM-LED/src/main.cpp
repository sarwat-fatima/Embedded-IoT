#include <Arduino.h>

// PWM Example – LED Brightness Fade
/*
Name: Sarwat Fatima
Reg no.: 23-NTU-CS-1092
Class: BSCS-5-A
*/
#include <Arduino.h>
#define LED_PIN 18
#define PWM_CH 0
#define FREQ 5000
#define RES 8
void setup() {
  ledcSetup(PWM_CH, FREQ, RES);
  ledcAttachPin(LED_PIN, PWM_CH);
}
void loop() {
  for (int d=0; d<=255; d++) { 
ledcWrite(PWM_CH, d); delay(10); }
  for (int d=255; d>=0; d--) { 
ledcWrite(PWM_CH, d); delay(10); }
}