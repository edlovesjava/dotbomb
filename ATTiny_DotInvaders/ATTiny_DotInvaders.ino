// DOTinvaders v2 - Wave Invasion
// Multiple invaders with staggered descent and trickle spawn

#include <avr/pgmspace.h>

// MAX7219 Register Addresses
#define MAX7219_REG_NOOP 0x00
#define MAX7219_REG_DECODE_MODE 0x09
#define MAX7219_REG_INTENSITY 0x0A
#define MAX7219_REG_SCAN_LIMIT 0x0B
#define MAX7219_REG_SHUTDOWN 0x0C
#define MAX7219_REG_DISPLAY_TEST 0x0F

// Pin definitions
const uint8_t DIN = 0;
const uint8_t CLK = 1;
const uint8_t CS = 2;
const uint8_t RIGHT_BUTTON = 3;
const uint8_t LEFT_BUTTON = 4;

// Game configuration
#define STARTING_LIVES 3
#define MAX_INVADERS 8
#define NUM_SPEED_LEVELS 8

// Speed levels (drop interval in ms) - stored in PROGMEM
const uint16_t speedLevels[] PROGMEM = {
  400, 350, 300, 260, 230, 200, 180, 160
};

// Wave configuration (invaders per wave) - stored in PROGMEM
const uint8_t waveInvaders[] PROGMEM = {
  3, 4, 5, 5, 6, 6, 7, 8  // Waves 1-8, wave 9+ stays at 8
};

// Spawn delay per wave (ms) - stored in PROGMEM
const uint16_t waveSpawnDelay[] PROGMEM = {
  600, 550, 500, 450, 400, 350, 300, 250  // Gets faster each wave
};

// Invader structure
struct Invader {
  int8_t col;           // -1 = inactive, 0-7 = column
  uint8_t row;          // 0-7 row position
  uint16_t dropTimer;   // Individual drop timer
  uint16_t dropDelay;   // Randomized drop interval
};

// Game state
byte matrix[8];
uint8_t gunPos = 3;
Invader invaders[MAX_INVADERS];
bool bulletActive = false;
uint8_t bulletCol = 0;
uint8_t bulletRow = 0;
uint16_t score = 0;

// Wave state
uint8_t waveNumber = 1;
uint8_t invadersToSpawn = 0;    // How many left to spawn this wave
uint8_t invadersActive = 0;     // Currently on screen
uint16_t spawnTimer = 0;        // Timer for trickle spawn

// Lives and progression
uint8_t lives = STARTING_LIVES;

// Timing
uint16_t bulletMoveCounter = 0;
bool lastFireState = false;

// Key repeat
uint16_t keyHoldTimer = 0;
uint8_t heldButton = 0;
const uint16_t KEY_REPEAT_DELAY = 250;
const uint16_t KEY_REPEAT_RATE = 50;

// Button state
bool lastLeftPressed = false;
bool lastRightPressed = false;
bool chordedThisPress = false;

// Send byte via bit-banged SPI
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

// Update display
void updateDisplay() {
  for (uint8_t row = 0; row < 8; row++) {
    sendCmd(row + 1, matrix[row]);
  }
}

// Get speed level for current wave
uint8_t getSpeedLevel() {
  if (waveNumber >= NUM_SPEED_LEVELS) return NUM_SPEED_LEVELS - 1;
  return waveNumber - 1;
}

// Get base drop interval for current wave
uint16_t getBaseDropInterval() {
  uint8_t idx = getSpeedLevel();
  return pgm_read_word(&speedLevels[idx]);
}

// Get spawn delay for current wave
uint16_t getSpawnDelay() {
  uint8_t idx = waveNumber - 1;
  if (idx >= NUM_SPEED_LEVELS) idx = NUM_SPEED_LEVELS - 1;
  return pgm_read_word(&waveSpawnDelay[idx]);
}

// Get invader count for wave
uint8_t getWaveInvaderCount() {
  uint8_t idx = waveNumber - 1;
  if (idx >= NUM_SPEED_LEVELS) idx = NUM_SPEED_LEVELS - 1;
  return pgm_read_byte(&waveInvaders[idx]);
}

// Flash effect for wave start
void flashWaveStart() {
  // Show wave number as columns lit
  for (uint8_t i = 0; i < 8; i++) matrix[i] = 0;
  uint8_t cols = waveNumber;
  if (cols > 8) cols = 8;
  for (uint8_t c = 0; c < cols; c++) {
    for (uint8_t r = 0; r < 8; r++) {
      matrix[r] |= (1 << c);
    }
  }
  updateDisplay();
  delay(500);

  // Flash
  for (uint8_t i = 0; i < 2; i++) {
    for (uint8_t r = 0; r < 8; r++) sendCmd(r + 1, 0xFF);
    delay(100);
    for (uint8_t r = 0; r < 8; r++) sendCmd(r + 1, 0x00);
    delay(100);
  }
}

// Initialize a new wave
void startWave() {
  // Clear all invaders
  for (uint8_t i = 0; i < MAX_INVADERS; i++) {
    invaders[i].col = -1;
  }

  invadersToSpawn = getWaveInvaderCount();
  invadersActive = 0;
  spawnTimer = 0;

  flashWaveStart();
}

// Find empty invader slot
int8_t findEmptySlot() {
  for (uint8_t i = 0; i < MAX_INVADERS; i++) {
    if (invaders[i].col == -1) return i;
  }
  return -1;
}

// Check if column is occupied at row 0
bool columnOccupied(uint8_t col) {
  for (uint8_t i = 0; i < MAX_INVADERS; i++) {
    if (invaders[i].col == col && invaders[i].row <= 1) {
      return true;
    }
  }
  return false;
}

// Spawn single invader (trickle)
void spawnOneInvader() {
  if (invadersToSpawn == 0) return;

  int8_t slot = findEmptySlot();
  if (slot == -1) return;

  // Find unoccupied column
  uint8_t col;
  uint8_t attempts = 0;
  do {
    col = random(0, 8);
    attempts++;
  } while (columnOccupied(col) && attempts < 16);

  invaders[slot].col = col;
  invaders[slot].row = 0;
  invaders[slot].dropTimer = 0;
  // Randomize drop delay slightly for stagger effect
  uint16_t baseDelay = getBaseDropInterval();
  invaders[slot].dropDelay = baseDelay + random(0, baseDelay / 2);

  invadersToSpawn--;
  invadersActive++;
}

// Show remaining lives
void showLivesRemaining() {
  for (uint8_t i = 0; i < lives; i++) {
    for (uint8_t r = 0; r < 8; r++) matrix[r] = 0xFF;
    updateDisplay();
    delay(200);
    for (uint8_t r = 0; r < 8; r++) matrix[r] = 0x00;
    updateDisplay();
    delay(200);
  }
  delay(300);
}

// Forward declaration
void gameOver();

// Deactivate invader that reached bottom
void invaderReachedBottom(uint8_t idx) {
  // Flash the row
  for (uint8_t i = 0; i < 3; i++) {
    sendCmd(8, 0xFF);  // Row 7
    delay(50);
    sendCmd(8, 0x00);
    delay(50);
  }

  invaders[idx].col = -1;
  invadersActive--;

  lives--;
  if (lives == 0) {
    gameOver();
  } else {
    showLivesRemaining();
  }
}

// Check if wave is complete
bool waveComplete() {
  return (invadersToSpawn == 0 && invadersActive == 0);
}

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
  sendCmd(MAX7219_REG_INTENSITY, 0x08);
  sendCmd(MAX7219_REG_SHUTDOWN, 0x01);

  // Clear display
  for (uint8_t i = 0; i < 8; i++) matrix[i] = 0;
  updateDisplay();

  randomSeed(analogRead(A0) ^ millis());

  showLivesRemaining();
  startWave();
}

void loop() {
  if (lives == 0) return;  // Game over state

  uint16_t bulletMoveInterval = 30;

  // Clear matrix
  for (uint8_t i = 0; i < 8; i++) matrix[i] = 0;

  // Draw gun
  matrix[7] |= (1 << gunPos);

  // Draw all active invaders
  for (uint8_t i = 0; i < MAX_INVADERS; i++) {
    if (invaders[i].col >= 0 && invaders[i].row < 8) {
      matrix[invaders[i].row] |= (1 << invaders[i].col);
    }
  }

  // Draw bullet
  if (bulletActive && bulletRow < 8) {
    matrix[bulletRow] |= (1 << bulletCol);
  }

  updateDisplay();
  delay(1);

  // Button handling
  bool rightPressed = (digitalRead(RIGHT_BUTTON) == LOW);
  bool leftPressed = (digitalRead(LEFT_BUTTON) == LOW);
  bool firePressed = rightPressed && leftPressed;

  if (firePressed && !lastFireState && !bulletActive) {
    bulletActive = true;
    bulletCol = gunPos;
    bulletRow = 6;
    chordedThisPress = true;
  }
  lastFireState = firePressed;

  // Movement on release
  if (!firePressed && !chordedThisPress) {
    if (lastLeftPressed && !leftPressed && !rightPressed) {
      if (gunPos < 7) gunPos++;
      heldButton = 0;
      keyHoldTimer = 0;
    }
    if (lastRightPressed && !rightPressed && !leftPressed) {
      if (gunPos > 0) gunPos--;
      heldButton = 0;
      keyHoldTimer = 0;
    }
  }

  if (!leftPressed && !rightPressed) {
    chordedThisPress = false;
  }

  // Key repeat
  if (!firePressed) {
    uint8_t currentButton = 0;
    if (leftPressed && !rightPressed) currentButton = 1;
    else if (rightPressed && !leftPressed) currentButton = 2;

    if (currentButton != 0) {
      if (currentButton == heldButton) {
        keyHoldTimer++;
        if (keyHoldTimer == KEY_REPEAT_DELAY) {
          if (currentButton == 1 && gunPos < 7) gunPos++;
          else if (currentButton == 2 && gunPos > 0) gunPos--;
        } else if (keyHoldTimer > KEY_REPEAT_DELAY) {
          if ((keyHoldTimer - KEY_REPEAT_DELAY) % KEY_REPEAT_RATE == 0) {
            if (currentButton == 1 && gunPos < 7) gunPos++;
            else if (currentButton == 2 && gunPos > 0) gunPos--;
          }
        }
      } else {
        heldButton = currentButton;
        keyHoldTimer = 0;
      }
    } else {
      heldButton = 0;
      keyHoldTimer = 0;
    }
  }

  lastLeftPressed = leftPressed;
  lastRightPressed = rightPressed;

  // Trickle spawn
  if (invadersToSpawn > 0) {
    spawnTimer++;
    if (spawnTimer >= getSpawnDelay()) {
      spawnTimer = 0;
      spawnOneInvader();
    }
  }

  // Move invaders (staggered)
  for (uint8_t i = 0; i < MAX_INVADERS; i++) {
    if (invaders[i].col >= 0) {
      invaders[i].dropTimer++;
      if (invaders[i].dropTimer >= invaders[i].dropDelay) {
        invaders[i].dropTimer = 0;
        invaders[i].row++;

        // Check if reached bottom
        if (invaders[i].row >= 7) {
          invaderReachedBottom(i);
          if (lives == 0) return;
        }
      }
    }
  }

  // Move bullet
  if (bulletActive) {
    bulletMoveCounter++;
    if (bulletMoveCounter >= bulletMoveInterval) {
      bulletMoveCounter = 0;
      if (bulletRow > 0) {
        bulletRow--;
      } else {
        bulletActive = false;
      }
    }
  }

  // Collision detection - check bullet against all invaders
  if (bulletActive) {
    for (uint8_t i = 0; i < MAX_INVADERS; i++) {
      if (invaders[i].col >= 0 &&
          bulletRow == invaders[i].row &&
          bulletCol == invaders[i].col) {
        // Hit!
        bulletActive = false;
        score += waveNumber;  // Wave multiplier

        // Flash hit
        for (uint8_t f = 0; f < 3; f++) {
          sendCmd(invaders[i].row + 1, 0xFF);
          delay(50);
          sendCmd(invaders[i].row + 1, 0x00);
          delay(50);
        }

        invaders[i].col = -1;  // Deactivate
        invadersActive--;
        break;
      }
    }
  }

  // Check wave complete
  if (waveComplete()) {
    waveNumber++;
    delay(500);
    startWave();
  }
}

void gameOver() {
  // Flash
  for (uint8_t f = 0; f < 4; f++) {
    for (uint8_t i = 0; i < 8; i++) matrix[i] = 0xFF;
    updateDisplay();
    delay(150);
    for (uint8_t i = 0; i < 8; i++) matrix[i] = 0x00;
    updateDisplay();
    delay(150);
  }

  // Show wave reached (columns = wave, max 8)
  for (uint8_t i = 0; i < 8; i++) matrix[i] = 0;
  uint8_t waveDisplay = waveNumber;
  if (waveDisplay > 8) waveDisplay = 8;
  for (uint8_t c = 0; c < waveDisplay; c++) {
    for (uint8_t r = 0; r < 8; r++) {
      matrix[r] |= (1 << c);
    }
  }
  updateDisplay();
  delay(2000);

  // Reset
  for (uint8_t i = 0; i < 8; i++) matrix[i] = 0;
  updateDisplay();

  score = 0;
  lives = STARTING_LIVES;
  waveNumber = 1;
  gunPos = 3;
  bulletActive = false;
  bulletMoveCounter = 0;
  keyHoldTimer = 0;
  heldButton = 0;
  lastLeftPressed = false;
  lastRightPressed = false;
  chordedThisPress = false;

  for (uint8_t i = 0; i < MAX_INVADERS; i++) {
    invaders[i].col = -1;
  }

  showLivesRemaining();
  startWave();
}
