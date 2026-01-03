/*
 * DotInvaders - Hardware Test Sketch
 * 
 * Use this sketch to verify your hardware is correctly wired
 * before uploading the main game.
 * 
 * This will test:
 * 1. MAX7219 LED Matrix display
 * 2. All three buttons
 * 3. Basic functionality
 */

#include "LedControl.h"

// Pin definitions (same as main game)
#define DIN_PIN 0
#define CLK_PIN 2
#define CS_PIN 1
#define BTN_LEFT 3
#define BTN_RIGHT 4
#define BTN_FIRE A1  // Pin 6 on ATTiny85

// LED Matrix controller
LedControl lc = LedControl(DIN_PIN, CLK_PIN, CS_PIN, 1);

// Test state
int testPhase = 0;
int cursorX = 3;
int cursorY = 3;

void setup() {
  // Initialize LED matrix
  lc.shutdown(0, false);
  lc.setIntensity(0, 8);
  lc.clearDisplay(0);
  
  // Initialize buttons
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_FIRE, INPUT_PULLUP);
  
  // Show startup pattern
  startupAnimation();
}

void loop() {
  switch (testPhase) {
    case 0:
      // Test all LEDs
      testAllLEDs();
      waitForFire();
      testPhase++;
      break;
      
    case 1:
      // Test individual LED control with buttons
      testButtonControl();
      break;
  }
}

void startupAnimation() {
  // Fill display one row at a time
  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 8; col++) {
      lc.setLed(0, row, col, true);
    }
    delay(100);
  }
  delay(500);
  
  // Clear display one row at a time
  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 8; col++) {
      lc.setLed(0, row, col, false);
    }
    delay(100);
  }
  delay(500);
}

void testAllLEDs() {
  // Light up all LEDs
  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 8; col++) {
      lc.setLed(0, row, col, true);
    }
  }
  
  delay(2000);
  
  // Flash 3 times
  for (int i = 0; i < 3; i++) {
    lc.clearDisplay(0);
    delay(200);
    for (int row = 0; row < 8; row++) {
      for (int col = 0; col < 8; col++) {
        lc.setLed(0, row, col, true);
      }
    }
    delay(200);
  }
  
  lc.clearDisplay(0);
}

void testButtonControl() {
  // Clear and show cursor
  lc.clearDisplay(0);
  lc.setLed(0, cursorY, cursorX, true);
  
  // Read buttons
  bool leftPressed = digitalRead(BTN_LEFT) == LOW;
  bool rightPressed = digitalRead(BTN_RIGHT) == LOW;
  bool firePressed = digitalRead(BTN_FIRE) == LOW;
  
  // Move cursor based on buttons
  if (leftPressed) {
    lc.setLed(0, cursorY, cursorX, false);
    cursorX--;
    if (cursorX < 0) cursorX = 7;
    lc.setLed(0, cursorY, cursorX, true);
    delay(150);
  }
  
  if (rightPressed) {
    lc.setLed(0, cursorY, cursorX, false);
    cursorX++;
    if (cursorX > 7) cursorX = 0;
    lc.setLed(0, cursorY, cursorX, true);
    delay(150);
  }
  
  if (firePressed) {
    lc.setLed(0, cursorY, cursorX, false);
    cursorY--;
    if (cursorY < 0) cursorY = 7;
    lc.setLed(0, cursorY, cursorX, true);
    delay(150);
  }
  
  delay(50);
}

void waitForFire() {
  // Flash current display while waiting
  bool state = true;
  while (digitalRead(BTN_FIRE) == HIGH) {
    // Flash all LEDs
    for (int row = 0; row < 8; row++) {
      for (int col = 0; col < 8; col++) {
        lc.setLed(0, row, col, state);
      }
    }
    state = !state;
    delay(300);
  }
  
  // Wait for button release
  while (digitalRead(BTN_FIRE) == LOW) {
    delay(10);
  }
  
  lc.clearDisplay(0);
  delay(200);
}
