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
};
```

### Tuning Table (stored in PROGMEM to save RAM)

```cpp
const WaveConfig waves[] PROGMEM = {
  // { invaders, dropInterval, spawnInterval, killsToAdvance }
  { 1, 400, 0,   5 },   // Wave 1: 1 invader, slow, 5 kills
  { 1, 350, 0,   5 },   // Wave 2: 1 invader, faster
  { 1, 300, 0,   5 },   // Wave 3: 1 invader, even faster
  { 2, 300, 500, 5 },   // Wave 4: 2 invaders, spawn every 500ms
  { 2, 250, 400, 5 },   // Wave 5: 2 invaders, faster
  { 3, 250, 350, 5 },   // Wave 6: 3 invaders
  { 3, 200, 300, 5 },   // Wave 7: 3 invaders, fast
  { 4, 200, 250, 0 },   // Wave 8+: 4 invaders, endgame (loop here)
};
#define NUM_WAVES 8
```

**Memory:** ~32 bytes in flash (PROGMEM), 0 bytes RAM

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
    if (invaders[i].active &&
        bulletRow == invaders[i].row &&
        bulletCol == invaders[i].col) {
      // Hit!
      invaders[i].active = false;
      bulletActive = false;
      score++;
      waveKills++;

      // Check wave advancement
      if (waveKills >= currentWave.killsToAdvance) {
        advanceWave();
      }
      break;
    }
  }
}
```

### Game Over Check
```cpp
void checkGameOver() {
  for (uint8_t i = 0; i < MAX_INVADERS; i++) {
    if (invaders[i].active && invaders[i].row >= 7) {
      gameOver();
      return;
    }
  }
}
```

---

## Tuning Parameters Summary

| Parameter | Description | Typical Range |
|-----------|-------------|---------------|
| `maxActiveInvaders` | Simultaneous invaders | 1-4 |
| `dropInterval` | Gravity (lower = faster) | 150-400 ms |
| `spawnInterval` | Time between spawns | 200-500 ms |
| `killsToAdvance` | Kills per wave | 3-10 |

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

### Phase 1: Multi-Invader Support
- [ ] Add invaders array
- [ ] Update draw loop for multiple invaders
- [ ] Update collision detection
- [ ] Update game over check
- [ ] Test with hardcoded 2 invaders

### Phase 2: Wave System
- [ ] Add wave configuration table
- [ ] Add spawn timer and logic
- [ ] Add wave advancement
- [ ] Test progression

### Phase 3: Tuning
- [ ] Playtest and adjust wave table values
- [ ] Balance difficulty curve
- [ ] Document final tuning values

---

## Questions for Review

1. **Max invaders:** Is 4 enough for endgame chaos, or should we support more?

2. **Spawn behavior:** When a new wave adds invaders, should they:
   - A) Spawn immediately (sudden difficulty spike)
   - B) Spawn gradually via spawnInterval (smoother)

3. **Wave looping:** At final wave, should it:
   - A) Loop the last wave forever
   - B) Keep incrementing speed infinitely
   - C) "Victory" state after X waves

4. **Visual feedback:** Flash or animation when advancing waves?

---

## Recommendation

Start with **Phase 1** - get multiple invaders working with hardcoded values. This validates the core mechanics before adding the wave system complexity.

Estimated code size increase: ~200-300 bytes (well within ATtiny85's 8KB flash).
