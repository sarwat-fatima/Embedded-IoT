#include <Arduino.h>
/*
Name: Sarwat Fatima
Reg no.: 23-NTU-CS-1092
Class: BSCS-5-A
*/
// Pins
#define BUZZER_PIN 27
#define LED1_PIN   18
#define LED2_PIN   19

// LEDC channels
#define CH_BUZZER  0   // buzzer channel
#define CH_LED1    1   // LED on pin 18
#define CH_LED2    2   // LED on pin 19

// Buzzer settings
#define BUZZER_BASE_FREQ 2000    // default base freq for ledcSetup on buzzer channel
#define BUZZER_RES       10      // resolution for buzzer channel (10-bit)

// LED settings
#define LED_FREQ  5000           // PWM frequency for LEDs
#define LED_RES   8              // 8-bit resolution for LEDs (0-255)

// LED fade step
const uint8_t LED_STEP = 2;      // brightness step per update (2 gives smoothish fade)

// Globals for LED states
int led1_brightness = 0;
int led2_brightness = 255;
int led1_dir = 1; // 1 = up, -1 = down
int led2_dir = -1;

void updateLEDsStep() {
  // update LED1
  if (led1_dir == 1) {
    led1_brightness += LED_STEP;
    if (led1_brightness >= 255) { led1_brightness = 255; led1_dir = -1; }
  } else {
    led1_brightness -= LED_STEP;
    if (led1_brightness <= 0) { led1_brightness = 0; led1_dir = 1; }
  }
  ledcWrite(CH_LED1, led1_brightness);

  // update LED2
  if (led2_dir == 1) {
    led2_brightness += LED_STEP;
    if (led2_brightness >= 255) { led2_brightness = 255; led2_dir = -1; }
  } else {
    led2_brightness -= LED_STEP;
    if (led2_brightness <= 0) { led2_brightness = 0; led2_dir = 1; }
  }
  ledcWrite(CH_LED2, led2_brightness);
}

void setup() {
  // Setup buzzer channel (use a base freq; ledcWriteTone will override)
  ledcSetup(CH_BUZZER, BUZZER_BASE_FREQ, BUZZER_RES);
  ledcAttachPin(BUZZER_PIN, CH_BUZZER);

  // Setup LED channels
  ledcSetup(CH_LED1, LED_FREQ, LED_RES);
  ledcAttachPin(LED1_PIN, CH_LED1);

  ledcSetup(CH_LED2, LED_FREQ, LED_RES);
  ledcAttachPin(LED2_PIN, CH_LED2);

  // initialize LEDs
  ledcWrite(CH_LED1, led1_brightness);
  ledcWrite(CH_LED2, led2_brightness);
}

void loop() {
  // --- 1. Simple beep pattern (3 beeps) ---
  for (int i = 0; i < 3; i++) {
    // start tone (uses ledcWriteTone, which sets PWM freq on that channel)
    ledcWriteTone(CH_BUZZER, 2000 + i * 400);
    // while tone plays, update LEDs several times to give concurrent fade effect
    for (int t = 0; t < 15; t++) { // ~15 * 10ms = 150ms
      updateLEDsStep();
      delay(10);
    }

    // stop tone
    ledcWrite(CH_BUZZER, 0);
    // short pause while still updating LEDs
    for (int t = 0; t < 15; t++) {
      updateLEDsStep();
      delay(10);
    }
  }

  delay(300); // small pause between sections

  // --- 2. Frequency sweep (400Hz → 3000Hz) ---
  for (int f = 400; f <= 3000; f += 100) {
    ledcWriteTone(CH_BUZZER, f);
    // update LEDs a bit while sweeping
    updateLEDsStep();
    delay(20);
  }
  // stop buzzer
  ledcWrite(CH_BUZZER, 0);

  delay(400);

  // --- 3. Short melody ---
  int melody[] = {262, 294, 330, 349, 392, 440, 494, 523};
  const int notes = sizeof(melody) / sizeof(melody[0]);
  for (int i = 0; i < notes; i++) {
    ledcWriteTone(CH_BUZZER, melody[i]);
    // while each note plays for 250ms, update LEDs in small steps
    for (int t = 0; t < 25; t++) { // 25 * 10ms = 250ms
      updateLEDsStep();
      delay(10);
    }
    // short silence between notes
    ledcWrite(CH_BUZZER, 0);
    for (int t = 0; t < 5; t++) {
      updateLEDsStep();
      delay(10);
    }
  }

  // stop buzzer to be safe
  ledcWrite(CH_BUZZER, 0);

  // pause before repeating the whole sequence
  for (int t = 0; t < 100; t++) { // ~1 second pause (100 * 10ms)
    updateLEDsStep();
    delay(10);
  }
}