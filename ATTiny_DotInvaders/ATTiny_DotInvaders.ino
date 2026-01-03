// MAX7219 Register Addresses
#define MAX7219_REG_NOOP 0x00
#define MAX7219_REG_DIGIT0 0x01
#define MAX7219_REG_DIGIT1 0x02
#define MAX7219_REG_DIGIT2 0x03
#define MAX7219_REG_DIGIT3 0x04
#define MAX7219_REG_DIGIT4 0x05
#define MAX7219_REG_DIGIT5 0x06
#define MAX7219_REG_DIGIT6 0x07
#define MAX7219_REG_DIGIT7 0x08

#define MAX7219_REG_DECODE_MODE 0x09
#define MAX7219_REG_INTENSITY 0x0A  // brightness 0–15
#define MAX7219_REG_SCAN_LIMIT 0x0B
#define MAX7219_REG_SHUTDOWN 0x0C
#define MAX7219_REG_DISPLAY_TEST 0x0F

// Pin definitions (based on dotris hardware)
const uint8_t DIN = 0;
const uint8_t CLK = 1;
const uint8_t CS = 2;
const uint8_t RIGHT_BUTTON = 3;
const uint8_t LEFT_BUTTON = 4;

// Game state
byte matrix[8];
uint8_t gunPos = 3;  // Gun position at bottom row (column 0-7)
uint8_t invaderCol = 0;  // Invader column position
uint8_t invaderRow = 0;  // Invader row position
bool bulletActive = false;
uint8_t bulletCol = 0;
uint8_t bulletRow = 0;
uint16_t score = 0;
uint8_t speed = 1;

// Timing variables
uint16_t invaderDropCounter = 0;
uint16_t bulletMoveCounter = 0;
bool lastFireState = false;

// Key repeat variables
uint16_t keyHoldTimer = 0;
uint8_t lastButtonState = 0;  // 0=none, 1=left, 2=right
const uint16_t KEY_REPEAT_DELAY = 250;  // Initial delay before repeat starts (ms)
const uint16_t KEY_REPEAT_RATE = 50;    // Repeat interval once holding (ms)

// A/B Test: Continue movement after fire-while-moving?
// true  = resume key repeat seamlessly after tap-to-fire
// false = reset key repeat delay after firing
const bool CONTINUE_MOVE_AFTER_FIRE = true;
uint8_t preFireButtonState = 0;  // Track which button was held before firing

// Send a byte to MAX7219 via bit-banged SPI
void sendByte(uint8_t b) {
  for (int i = 7; i >= 0; i--) {
    digitalWrite(CLK, LOW);
    digitalWrite(DIN, (b >> i) & 1);
    digitalWrite(CLK, HIGH);
  }
}

// Send command to MAX7219
void sendCmd(uint8_t addr, uint8_t data) {
  digitalWrite(CS, LOW);
  sendByte(addr);
  sendByte(data);
  digitalWrite(CS, HIGH);
}

// Update entire display from matrix buffer
void updateDisplay() {
  for (uint8_t row = 0; row < 8; row++) {
    sendCmd(row + 1, matrix[row]);
  }
}

// Initialize MAX7219
void setup() {
  pinMode(DIN, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(CS, OUTPUT);
  pinMode(RIGHT_BUTTON, INPUT_PULLUP);
  pinMode(LEFT_BUTTON, INPUT_PULLUP);

  // Initialize MAX7219
  sendCmd(MAX7219_REG_DISPLAY_TEST, 0x00);
  sendCmd(MAX7219_REG_DECODE_MODE, 0x00);
  sendCmd(MAX7219_REG_SCAN_LIMIT, 0x07);
  sendCmd(MAX7219_REG_INTENSITY, 0x08);  // brightness
  sendCmd(MAX7219_REG_SHUTDOWN, 0x01);

  // Clear display
  for (uint8_t i = 0; i < 8; i++) matrix[i] = 0;
  updateDisplay();

  // Initialize random seed using millis() for better randomness
  // The unpredictable power-on time provides entropy
  randomSeed(millis());
  
  // Spawn first invader
  spawnInvader();
}

// Spawn a new invader at a random column
void spawnInvader() {
  invaderCol = random(0, 8);
  invaderRow = 0;
}

// Main game loop
void loop() {
  uint16_t invaderDropInterval = 300 - (speed * 10);  // Gets faster with score
  uint16_t bulletMoveInterval = 30;  // Fast bullet movement
  
  // Clear matrix for fresh frame
  for (uint8_t i = 0; i < 8; i++) matrix[i] = 0;
  
  // Draw gun at bottom row
  matrix[7] |= (1 << gunPos);
  
  // Draw invader if still alive
  if (invaderRow < 8) {
    matrix[invaderRow] |= (1 << invaderCol);
  }
  
  // Draw bullet if active
  if (bulletActive) {
    if (bulletRow < 8) {
      matrix[bulletRow] |= (1 << bulletCol);
    }
  }
  
  // Update display
  updateDisplay();
  
  // Small delay for game timing
  delay(1);
  
  // Handle button input for gun movement and firing
  bool rightPressed = (digitalRead(RIGHT_BUTTON) == LOW);
  bool leftPressed = (digitalRead(LEFT_BUTTON) == LOW);

  // Chording both buttons fires a bullet
  bool firePressed = rightPressed && leftPressed;

  if (firePressed && !lastFireState && !bulletActive) {
    // Fire a bullet - remember what direction we were moving
    if (CONTINUE_MOVE_AFTER_FIRE) {
      preFireButtonState = lastButtonState;
    }
    bulletActive = true;
    bulletCol = gunPos;
    bulletRow = 6;  // Start just above the gun
  }
  lastFireState = firePressed;

  // Handle movement with key repeat (only if not firing)
  if (!firePressed) {
    uint8_t currentButton = 0;
    if (leftPressed) currentButton = 1;
    else if (rightPressed) currentButton = 2;

    if (currentButton != 0) {
      bool shouldMove = false;

      // Check if we're resuming movement after firing
      bool resumingAfterFire = CONTINUE_MOVE_AFTER_FIRE &&
                                preFireButtonState != 0 &&
                                currentButton == preFireButtonState;

      if (resumingAfterFire) {
        // Continue where we left off - don't reset timer, don't move immediately
        // Just keep counting from where we were
        preFireButtonState = 0;  // Clear the flag
        keyHoldTimer++;
        if (keyHoldTimer >= KEY_REPEAT_DELAY) {
          if ((keyHoldTimer - KEY_REPEAT_DELAY) % KEY_REPEAT_RATE == 0) {
            shouldMove = true;
          }
        }
      } else if (currentButton != lastButtonState) {
        // New button press - move immediately
        shouldMove = true;
        keyHoldTimer = 0;
        preFireButtonState = 0;
      } else {
        // Button held - check for repeat
        keyHoldTimer++;
        if (keyHoldTimer == KEY_REPEAT_DELAY) {
          // Initial repeat after delay
          shouldMove = true;
        } else if (keyHoldTimer > KEY_REPEAT_DELAY) {
          // Continuous repeat at faster rate
          if ((keyHoldTimer - KEY_REPEAT_DELAY) % KEY_REPEAT_RATE == 0) {
            shouldMove = true;
          }
        }
      }

      if (shouldMove) {
        if (currentButton == 1 && gunPos < 7) {
          gunPos++;  // Move left
        } else if (currentButton == 2 && gunPos > 0) {
          gunPos--;  // Move right
        }
      }

      lastButtonState = currentButton;
    } else {
      // No button pressed - reset state
      lastButtonState = 0;
      keyHoldTimer = 0;
      preFireButtonState = 0;
    }
  }
  
  // Move invader down
  invaderDropCounter++;
  if (invaderDropCounter >= invaderDropInterval) {
    invaderDropCounter = 0;
    invaderRow++;
  }
  
  // Move bullet up
  if (bulletActive) {
    bulletMoveCounter++;
    if (bulletMoveCounter >= bulletMoveInterval) {
      bulletMoveCounter = 0;
      
      if (bulletRow > 0) {
        bulletRow--;
      } else {
        // Bullet went off screen
        bulletActive = false;
      }
    }
  }
  
  // Check for collision (bullet hits invader)
  if (bulletActive && bulletRow == invaderRow && bulletCol == invaderCol) {
    // Hit!
    bulletActive = false;
    score++;
    speed++;
    if (speed > 20) speed = 20;
    
    // Flash the hit
    for (uint8_t i = 0; i < 3; i++) {
      sendCmd(invaderRow + 1, 0xFF);
      delay(50);
      sendCmd(invaderRow + 1, 0x00);
      delay(50);
    }
    
    // Spawn new invader
    spawnInvader();
    invaderDropCounter = 0;
  }
  
  // Check if invader reached the bottom (game over)
  if (invaderRow >= 7) {
    gameOver();
    return;
  }
}

// Game over sequence
void gameOver() {
  // Flash entire display
  for (uint8_t i = 0; i < 8; i++) matrix[i] = 0xFF;
  updateDisplay();
  delay(200);
  for (uint8_t i = 0; i < 8; i++) matrix[i] = 0x00;
  updateDisplay();
  delay(200);
  for (uint8_t i = 0; i < 8; i++) matrix[i] = 0xFF;
  updateDisplay();
  delay(200);
  for (uint8_t i = 0; i < 8; i++) matrix[i] = 0x00;
  updateDisplay();
  
  // Display score pattern (simplified - number of rows lit = score)
  uint8_t scoreDisplay = score;
  if (scoreDisplay > 8) scoreDisplay = 8;
  for (uint8_t i = 0; i < scoreDisplay; i++) {
    matrix[7 - i] = 0xFF;
  }
  updateDisplay();
  delay(2000);
  
  // Reset game
  for (uint8_t i = 0; i < 8; i++) matrix[i] = 0;
  updateDisplay();
  score = 0;
  speed = 1;
  gunPos = 3;
  bulletActive = false;
  invaderDropCounter = 0;
  bulletMoveCounter = 0;
  keyHoldTimer = 0;
  lastButtonState = 0;
  preFireButtonState = 0;
  spawnInvader();
  delay(500);
}
