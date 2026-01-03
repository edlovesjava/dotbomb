# Wave System Design Plan

## ATtiny85 Hardware Budget

### Specifications
| Resource | Total | Current Use | Available | After Wave System |
|----------|-------|-------------|-----------|-------------------|
| **Flash** | 8,192 bytes | ~2,500* | ~5,700 | ~3,000 (+500) |
| **SRAM** | 512 bytes | ~86 | ~426 | ~140 (+54) |
| **EEPROM** | 512 bytes | 0 | 512 | 0 (or 2 for high score) |
| **CPU** | 8 MHz | ~5% | 95% | ~8% |
| **Power** | Battery | Low | N/A | Minimal impact |

*Estimated - actual requires compilation with avr-gcc

### Current SRAM Usage Analysis

```
Global Variables (26 bytes):
  matrix[8]           8 bytes   Display buffer
  gunPos              1 byte    Gun position
  invaderCol          1 byte    Invader X
  invaderRow          1 byte    Invader Y
  bulletActive        1 byte    Bullet state
  bulletCol           1 byte    Bullet X
  bulletRow           1 byte    Bullet Y
  score               2 bytes   Current score
  speed               1 byte    Current speed level
  invaderDropCounter  2 bytes   Drop timing
  bulletMoveCounter   2 bytes   Bullet timing
  lastFireState       1 byte    Fire debounce
  keyHoldTimer        2 bytes   Key repeat timing
  lastButtonState     1 byte    Movement state
  preFireButtonState  1 byte    Fire-while-moving

Constants (may be in flash): ~10 bytes
Stack (function calls):      ~50 bytes
                             ___________
Estimated current total:     ~86 bytes (17% of 512)
```

### Wave System Additions

```
New Global Variables (+20 bytes):
  invaders[4] (col, row, active)  12 bytes  Multi-invader array
  currentWave                      1 byte   Wave index
  waveKills                        1 byte   Kills in current wave
  spawnCounter                     2 bytes  Spawn timing
  spawnInterval (runtime)          2 bytes  Current spawn rate
  dropInterval (runtime)           2 bytes  Current gravity

Removed Variables (-4 bytes):
  invaderCol                      -1 byte   Replaced by array
  invaderRow                      -1 byte   Replaced by array
  speed                           -1 byte   Replaced by wave system
  (invaderDropInterval local)     -2 bytes  Now global

Wave Config Table (PROGMEM = Flash, 0 SRAM):
  8 waves × 6 bytes = 48 bytes Flash

Net SRAM change: +16 bytes
New total: ~102 bytes (20% of 512)
```

### CPU Impact Analysis

| Operation | Current | After | Notes |
|-----------|---------|-------|-------|
| Draw loop | O(1) | O(4) | Loop through 4 invaders |
| Collision | O(1) | O(4) | Check each invader |
| Spawn check | N/A | O(4) | Count active invaders |
| Main loop | ~1ms | ~1.5ms | Still well under 16ms frame |

**Verdict:** Negligible CPU impact. 8MHz handles this easily.

### Power Considerations

- LED matrix is the main power draw (controlled by MAX7219)
- Additional code has minimal power impact
- No sleep mode changes needed
- Battery life unchanged

---

## Current State
- Single invader at a time
- Linear speed increase with each kill
- No progression phases

## Desired Behavior
1. Start with 1 invader, moderate pace
2. Every 3-5 kills, increase pace
3. After N kills, add a second simultaneous invader
4. Continue ramping: more invaders + faster pace
5. Endgame: multiple fast invaders

---

## Design Decisions

| Question | Decision |
|----------|----------|
| Max invaders | **4** (constant `MAX_INVADERS`) |
| Wave advance effect | **Yes** - flash effect on level up |
| Lives system | **Yes** - 1-3 lives (configurable) |
| Score multiplier | **Yes** - increases with wave level |
| New wave spawn | **Gradual** - via spawnInterval, not sudden burst |
| Endgame | **Loop final wave** with max difficulty |

---

## New Features

### Lives System

```cpp
#define STARTING_LIVES 3
#define MAX_LIVES 3

uint8_t lives = STARTING_LIVES;  // 1 byte SRAM

void loseLife() {
  lives--;
  if (lives == 0) {
    gameOver();
  } else {
    // Flash remaining lives, clear invaders, brief pause
    showLivesRemaining();
    clearAllInvaders();
    delay(1000);
  }
}
```

**Display lives:** Flash pattern at game start, or use corner LEDs.

### Score Multiplier

```cpp
// Add to WaveConfig or calculate from wave number
uint8_t scoreMultiplier = 1;  // 1 byte SRAM

// On hit:
score += scoreMultiplier;  // Instead of score++

// On wave advance:
scoreMultiplier = currentWave + 1;  // Wave 1 = 1x, Wave 2 = 2x, etc.
```

**Alternative:** Store multiplier in wave config table for fine control.

### Wave Advance Flash Effect

```cpp
void advanceWave() {
  currentWave++;
  waveKills = 0;

  // Flash effect - quick strobe
  for (uint8_t i = 0; i < 3; i++) {
    for (uint8_t row = 0; row < 8; row++) {
      sendCmd(row + 1, 0xFF);
    }
    delay(50);
    for (uint8_t row = 0; row < 8; row++) {
      sendCmd(row + 1, 0x00);
    }
    delay(50);
  }

  // Load new wave config from PROGMEM
  loadWaveConfig(currentWave);
}
```

---

## Updated SRAM Budget

```
Wave System Variables:
  invaderCols[4]       4 bytes   Invader X positions
  invaderRows[4]       4 bytes   Invader Y positions (255=inactive)
  currentWave          1 byte    Current wave index
  waveKills            1 byte    Kills in current wave
  spawnCounter         2 bytes   Spawn timing
  dropInterval         2 bytes   Current gravity
  spawnInterval        2 bytes   Current spawn rate
  lives                1 byte    Remaining lives
  scoreMultiplier      1 byte    Current score multiplier
                      ________
  Subtotal:           18 bytes

Removed:
  invaderCol          -1 byte
  invaderRow          -1 byte
  speed               -1 byte
                      ________
  Net change:        +15 bytes

New total: ~101 bytes (20% of 512 SRAM) ✓
```

---

## Proposed Architecture

### Invader Storage

**Option A: Struct with active flag (12 bytes)**
```cpp
#define MAX_INVADERS 4

struct Invader {
  uint8_t col;    // 0-7
  uint8_t row;    // 0-7
  bool active;    // true/false
};
Invader invaders[MAX_INVADERS];
```

**Option B: Packed, no active flag (8 bytes) - RECOMMENDED**
```cpp
#define MAX_INVADERS 4
#define INVADER_INACTIVE 255

uint8_t invaderCols[MAX_INVADERS];  // 4 bytes
uint8_t invaderRows[MAX_INVADERS];  // 4 bytes (255 = inactive)
```

Benefits of Option B:
- 4 bytes smaller (8 vs 12)
- Simpler iteration (no struct access)
- `row == 255` means inactive (can't be on row 255 anyway)
- Parallel arrays are cache-friendly (though irrelevant on ATtiny)

### Wave Configuration

```cpp
struct WaveConfig {
  uint8_t maxActiveInvaders;  // 1-4 invaders allowed on screen
  uint16_t dropInterval;      // ms between drops (gravity)
  uint16_t spawnInterval;     // ms between new spawns
  uint8_t killsToAdvance;     // kills needed to reach next wave
  uint8_t scoreMultiplier;    // points per kill this wave
};
```

### Tuning Table (stored in PROGMEM to save RAM)

```cpp
const WaveConfig waves[] PROGMEM = {
  // { invaders, drop, spawn, kills, multiplier }
  { 1, 400,   0, 5, 1 },   // Wave 1: 1 invader, slow, 1x
  { 1, 350,   0, 5, 1 },   // Wave 2: 1 invader, faster, 1x
  { 1, 300,   0, 5, 2 },   // Wave 3: 1 invader, fast, 2x
  { 2, 300, 600, 5, 2 },   // Wave 4: 2 invaders, 2x
  { 2, 250, 500, 5, 3 },   // Wave 5: 2 invaders, faster, 3x
  { 3, 250, 400, 5, 3 },   // Wave 6: 3 invaders, 3x
  { 3, 200, 350, 5, 4 },   // Wave 7: 3 invaders, fast, 4x
  { 4, 200, 300, 0, 5 },   // Wave 8+: 4 invaders, endgame, 5x
};
#define NUM_WAVES 8
```

**Memory:** ~48 bytes in flash (PROGMEM), 0 bytes RAM

---

## Game Loop Changes

### Spawn Logic
```cpp
void trySpawnInvader() {
  // Count active invaders
  uint8_t activeCount = 0;
  for (uint8_t i = 0; i < MAX_INVADERS; i++) {
    if (invaders[i].active) activeCount++;
  }

  // Check if we can spawn (under limit and spawn timer ready)
  if (activeCount < currentWave.maxActiveInvaders && spawnTimerReady) {
    // Find inactive slot and spawn
    for (uint8_t i = 0; i < MAX_INVADERS; i++) {
      if (!invaders[i].active) {
        invaders[i].col = random(0, 8);
        invaders[i].row = 0;
        invaders[i].active = true;
        break;
      }
    }
  }
}
```

### Collision Detection
```cpp
void checkCollisions() {
  if (!bulletActive) return;

  for (uint8_t i = 0; i < MAX_INVADERS; i++) {
    if (invaderRows[i] != INVADER_INACTIVE &&
        bulletRow == invaderRows[i] &&
        bulletCol == invaderCols[i]) {
      // Hit!
      invaderRows[i] = INVADER_INACTIVE;
      bulletActive = false;
      score += scoreMultiplier;  // Apply wave multiplier
      waveKills++;

      // Flash hit effect
      flashHit(invaderRows[i]);

      // Check wave advancement
      if (waveKills >= currentWaveKillsToAdvance) {
        advanceWave();
      }
      break;
    }
  }
}
```

### Game Over Check (with Lives)
```cpp
void checkInvaderReachedBottom() {
  for (uint8_t i = 0; i < MAX_INVADERS; i++) {
    if (invaderRows[i] != INVADER_INACTIVE && invaderRows[i] >= 7) {
      // Invader reached bottom - lose a life
      loseLife();
      return;
    }
  }
}

void loseLife() {
  lives--;

  // Clear all invaders
  for (uint8_t i = 0; i < MAX_INVADERS; i++) {
    invaderRows[i] = INVADER_INACTIVE;
  }

  if (lives == 0) {
    gameOver();
  } else {
    // Show remaining lives (flash N times)
    for (uint8_t i = 0; i < lives; i++) {
      for (uint8_t row = 0; row < 8; row++) matrix[row] = 0xFF;
      updateDisplay();
      delay(200);
      for (uint8_t row = 0; row < 8; row++) matrix[row] = 0x00;
      updateDisplay();
      delay(200);
    }
    delay(500);
    // Resume - invaders will spawn via normal spawn logic
  }
}
```

---

## Tuning Parameters Summary

| Parameter | Description | Typical Range |
|-----------|-------------|---------------|
| `MAX_INVADERS` | Max simultaneous invaders | 4 (constant) |
| `STARTING_LIVES` | Lives at game start | 1-3 |
| `maxActiveInvaders` | Invaders allowed this wave | 1-4 |
| `dropInterval` | Gravity (lower = faster) | 150-400 ms |
| `spawnInterval` | Time between spawns | 200-600 ms |
| `killsToAdvance` | Kills to next wave | 3-10 |
| `scoreMultiplier` | Points per kill | 1-5x |

---

## Budget Validation Checklist

Before implementing, verify:

- [x] **SRAM:** 102 bytes used / 512 available = 20% ✓
- [x] **Flash:** ~3,000 bytes / 8,192 available = 37% ✓
- [x] **EEPROM:** 0-2 bytes (optional high score) ✓
- [x] **CPU:** <10% utilization at 8MHz ✓
- [x] **Power:** No additional drain beyond existing ✓

### Risk Assessment

| Risk | Likelihood | Mitigation |
|------|------------|------------|
| Stack overflow | Low | Keep functions shallow, avoid recursion |
| Flash overflow | Very Low | 5KB+ headroom |
| Timing issues | Low | Test on hardware, adjust intervals |
| PROGMEM read issues | Medium | Use pgm_read_* macros correctly |

---

## Implementation Phases

### Phase 0: Single-Invader Core Gameplay (BUILD & TEST FIRST)

Get the base game feeling right before adding multi-invader complexity.

**Constraints:**
- Single invader at a time
- Single bullet active (miss = wait for bullet to exit screen)
- Random spawn column for each new invader

**Features to add:**
- [ ] Lives system (3 lives)
- [ ] Incremental speed increase (every N kills)
- [ ] Score multiplier (increases with speed/level)
- [ ] Level-up flash effect
- [ ] Difficulty curve tuning

**Difficulty Progression (single invader):**
```cpp
// Speed levels for single-invader phase
const uint16_t speedLevels[] PROGMEM = {
  400,  // Level 1: slow
  350,  // Level 2
  300,  // Level 3
  260,  // Level 4
  230,  // Level 5
  200,  // Level 6
  180,  // Level 7
  160,  // Level 8+: max speed
};
#define KILLS_PER_LEVEL 5
#define NUM_SPEED_LEVELS 8
```

**Score multiplier:** Level N = Nx points per kill

**Missed shot penalty:** Must wait ~200ms for bullet to travel off screen before firing again. This creates tension - do you take the shot or wait for better alignment?

**SRAM additions for Phase 0:**
```
lives              1 byte
currentLevel       1 byte
levelKills         1 byte
scoreMultiplier    1 byte (or calculate from level)
                   _______
Net addition:      ~3-4 bytes
```

**Playtest goals:**
- [ ] Does speed curve feel fair?
- [ ] Is 3 lives too many/few?
- [ ] Does missed-shot-wait feel punishing or strategic?
- [ ] Is score multiplier motivating?

---

### Phase 1: Multi-Invader Support
- [ ] Add invaderCols[4] and invaderRows[4] arrays
- [ ] Update draw loop for multiple invaders
- [ ] Update collision detection
- [ ] Update game over check (basic, single life)
- [ ] Test with hardcoded 2 invaders

### Phase 2: Wave System + Lives
- [ ] Add wave configuration table (PROGMEM)
- [ ] Add spawn timer and logic
- [ ] Add wave advancement with flash effect
- [ ] Add lives system (lose life, flash count, reset invaders)
- [ ] Add score multiplier
- [ ] Test progression through waves

### Phase 3: Tuning & Polish
- [ ] Playtest and adjust wave table values
- [ ] Balance difficulty curve
- [ ] Tune spawn intervals for fun gameplay
- [ ] Document final tuning values
- [ ] Optional: High score to EEPROM

---

## Summary

### Development Approach: Iterative Build & Test

| Phase | Focus | Test Before Proceeding |
|-------|-------|------------------------|
| **Phase 0** | Single invader, lives, speed curve, multiplier | Core feel right? |
| Phase 1 | Multi-invader arrays, collision | Mechanics work? |
| Phase 2 | Wave config, spawning, transitions | Progression fun? |
| Phase 3 | Final tuning, polish | Ready for release? |

### Phase 0 Features (Single Invader)

| Feature | Status |
|---------|--------|
| Single invader, single bullet | Current behavior |
| Lives system (3 lives) | To implement |
| Incremental speed (8 levels) | To implement |
| Score multiplier (1-8x) | To implement |
| Level-up flash effect | To implement |
| Missed shot = wait for bullet | Current behavior |

**Hardware budget: ✓ All resources within limits**

---

## Next Step

**Approve to begin Phase 0 implementation:**
1. Add lives system
2. Add level-based speed progression
3. Add score multiplier
4. Add level-up flash effect
5. Playtest and tune

Estimated code size increase: ~150-200 bytes.
