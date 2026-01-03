/*
 * DotInvaders - Space Invaders for 8x8 LED Matrix
 * 
 * Hardware:
 * - ATTiny85 (or Arduino compatible)
 * - MAX7219 8x8 LED Matrix
 * - 3 buttons (Left, Right, Fire)
 * 
 * Wiring:
 * - MAX7219 DIN  -> Pin 0 (MOSI)
 * - MAX7219 CLK  -> Pin 2 (SCK)
 * - MAX7219 CS   -> Pin 1
 * - Button Left  -> Pin 3
 * - Button Right -> Pin 4
 * - Button Fire  -> Pin 5 (if using ATTiny85, use A1/Pin 6)
 */

#include "LedControl.h"

// Pin definitions
#define DIN_PIN 0
#define CLK_PIN 2
#define CS_PIN 1
#define BTN_LEFT 3
#define BTN_RIGHT 4
#define BTN_FIRE A1  // Pin 6 on ATTiny85

// Game constants
#define MATRIX_SIZE 8
#define GAME_SPEED 200
#define ENEMY_SPEED 500
#define BULLET_SPEED 100

// LED Matrix controller
LedControl lc = LedControl(DIN_PIN, CLK_PIN, CS_PIN, 1);

// Game state
enum GameState {
  GAME_START,
  GAME_PLAYING,
  GAME_OVER
};

GameState gameState = GAME_START;

// Player
int playerX = 3;  // Player position (0-7)
const int playerY = 7;  // Player always at bottom

// Bullet
int bulletX = -1;
int bulletY = -1;
bool bulletActive = false;

// Enemies (3 rows x 4 columns on 8x8 grid)
bool enemies[3][8];  // 3 rows of enemies
int enemyY = 0;  // Top position of enemy formation
int enemyDirection = 1;  // 1 = right, -1 = left
int enemyCount = 0;

// Timing
unsigned long lastEnemyMove = 0;
unsigned long lastBulletMove = 0;
unsigned long lastGameUpdate = 0;

// Score
int score = 0;
int lives = 3;

void setup() {
  // Initialize LED matrix
  lc.shutdown(0, false);
  lc.setIntensity(0, 8);
  lc.clearDisplay(0);
  
  // Initialize buttons
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_FIRE, INPUT_PULLUP);
  
  // Initialize game
  initGame();
}

void loop() {
  unsigned long currentTime = millis();
  
  switch (gameState) {
    case GAME_START:
      handleStart();
      break;
      
    case GAME_PLAYING:
      handleInput();
      
      // Update bullet
      if (currentTime - lastBulletMove > BULLET_SPEED) {
        updateBullet();
        lastBulletMove = currentTime;
      }
      
      // Update enemies
      if (currentTime - lastEnemyMove > ENEMY_SPEED) {
        updateEnemies();
        lastEnemyMove = currentTime;
      }
      
      // Check game over conditions
      if (enemyCount == 0) {
        // Win condition - restart with faster enemies
        initGame();
      }
      
      if (lives <= 0 || enemyY + 2 >= playerY) {
        gameState = GAME_OVER;
      }
      
      // Update display
      if (currentTime - lastGameUpdate > GAME_SPEED) {
        updateDisplay();
        lastGameUpdate = currentTime;
      }
      break;
      
    case GAME_OVER:
      handleGameOver();
      break;
  }
}

void initGame() {
  // Clear all
  lc.clearDisplay(0);
  
  // Initialize enemies (3 rows, spread across top)
  enemyCount = 0;
  enemyY = 0;
  enemyDirection = 1;
  
  for (int row = 0; row < 3; row++) {
    for (int col = 0; col < 8; col++) {
      // Create a pattern - enemy every other column in each row
      if (col % 2 == 0) {
        enemies[row][col] = true;
        enemyCount++;
      } else {
        enemies[row][col] = false;
      }
    }
  }
  
  // Reset player
  playerX = 3;
  bulletActive = false;
  bulletX = -1;
  bulletY = -1;
  
  // Reset score if starting new game
  if (gameState == GAME_START) {
    score = 0;
    lives = 3;
  }
  
  gameState = GAME_PLAYING;
  lastEnemyMove = millis();
  lastBulletMove = millis();
  lastGameUpdate = millis();
}

void handleStart() {
  // Show start animation
  for (int i = 0; i < 8; i++) {
    lc.setLed(0, i, i, true);
    delay(100);
  }
  delay(500);
  lc.clearDisplay(0);
  
  initGame();
}

void handleInput() {
  // Read buttons
  bool leftPressed = digitalRead(BTN_LEFT) == LOW;
  bool rightPressed = digitalRead(BTN_RIGHT) == LOW;
  bool firePressed = digitalRead(BTN_FIRE) == LOW;
  
  // Move player
  if (leftPressed && playerX > 0) {
    playerX--;
    delay(150);  // Debounce
  }
  
  if (rightPressed && playerX < 7) {
    playerX++;
    delay(150);  // Debounce
  }
  
  // Fire bullet
  if (firePressed && !bulletActive) {
    bulletActive = true;
    bulletX = playerX;
    bulletY = playerY - 1;
    delay(150);  // Debounce
  }
}

void updateBullet() {
  if (!bulletActive) return;
  
  // Move bullet up
  bulletY--;
  
  // Check if bullet hit enemy
  if (bulletY >= enemyY && bulletY < enemyY + 3) {
    int enemyRow = bulletY - enemyY;
    if (enemies[enemyRow][bulletX]) {
      enemies[enemyRow][bulletX] = false;
      enemyCount--;
      score++;
      bulletActive = false;
      bulletX = -1;
      bulletY = -1;
      return;
    }
  }
  
  // Check if bullet went off screen
  if (bulletY < 0) {
    bulletActive = false;
    bulletX = -1;
    bulletY = -1;
  }
}

void updateEnemies() {
  bool changeDirection = false;
  
  // Check if any enemy hit the edge
  for (int row = 0; row < 3; row++) {
    for (int col = 0; col < 8; col++) {
      if (enemies[row][col]) {
        int enemyX = col;
        if ((enemyDirection > 0 && enemyX >= 7) || 
            (enemyDirection < 0 && enemyX <= 0)) {
          changeDirection = true;
          break;
        }
      }
    }
    if (changeDirection) break;
  }
  
  if (changeDirection) {
    // Move down and change direction
    enemyY++;
    enemyDirection *= -1;
  } else {
    // Move horizontally
    if (enemyDirection > 0) {
      // Move right - scan from right to left to avoid overwriting
      for (int row = 0; row < 3; row++) {
        for (int col = 7; col >= 0; col--) {
          if (enemies[row][col]) {
            enemies[row][col] = false;
            if (col < 7) {
              enemies[row][col + 1] = true;
            }
          }
        }
      }
    } else {
      // Move left - scan from left to right to avoid overwriting
      for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 8; col++) {
          if (enemies[row][col]) {
            enemies[row][col] = false;
            if (col > 0) {
              enemies[row][col - 1] = true;
            }
          }
        }
      }
    }
  }
}

void updateDisplay() {
  // Clear display
  lc.clearDisplay(0);
  
  // Draw player
  lc.setLed(0, playerY, playerX, true);
  
  // Draw bullet
  if (bulletActive) {
    lc.setLed(0, bulletY, bulletX, true);
  }
  
  // Draw enemies
  for (int row = 0; row < 3; row++) {
    for (int col = 0; col < 8; col++) {
      if (enemies[row][col]) {
        int displayY = enemyY + row;
        if (displayY >= 0 && displayY < 8) {
          lc.setLed(0, displayY, col, true);
        }
      }
    }
  }
}

void handleGameOver() {
  // Flash display
  for (int i = 0; i < 3; i++) {
    lc.clearDisplay(0);
    delay(200);
    
    // Fill display
    for (int row = 0; row < 8; row++) {
      for (int col = 0; col < 8; col++) {
        lc.setLed(0, row, col, true);
      }
    }
    delay(200);
  }
  
  lc.clearDisplay(0);
  delay(1000);
  
  // Wait for fire button to restart
  while (digitalRead(BTN_FIRE) == HIGH) {
    // Show score pattern (simplified)
    delay(100);
  }
  
  // Restart game
  gameState = GAME_START;
  initGame();
}
