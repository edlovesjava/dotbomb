# Dot Invaders - Game Documentation

## Game Overview

Dot Invaders is a minimalist space shooter game inspired by the classic Space Invaders arcade game. The game runs on an ATtiny85 microcontroller with an 8x8 LED matrix display.

## Game Objective

Shoot down descending invader dots before they reach your gun at the bottom of the screen. Each successful hit increases your score and the game speed.

## Game Elements

### The Gun (Your Ship)
- **Position**: Bottom row (row 7) of the LED matrix
- **Representation**: A single lit LED dot
- **Movement**: Horizontal only (left and right)
- **Control**: Two buttons (left and right)

### The Invader
- **Position**: Starts at random column in top row (row 0)
- **Representation**: A single lit LED dot
- **Movement**: Descends vertically one row at a time
- **Behavior**: New invader spawns after each hit or game over

### The Bullet
- **Position**: Starts one row above the gun (row 6)
- **Representation**: A single lit LED dot
- **Movement**: Travels upward rapidly
- **Behavior**: Only one bullet can be active at a time

## Controls

| Input | Action |
|-------|--------|
| LEFT Button Only | Move gun right* |
| RIGHT Button Only | Move gun left* |
| BOTH Buttons (Chord) | Fire bullet |

*Note: Due to hardware wiring, the button labels may be reversed from the actual movement. This is consistent with the dotris hardware setup.

## Gameplay Mechanics

### Starting the Game
1. Power on the device
2. The game starts automatically
3. First invader appears at a random column in the top row
4. Gun appears in the center of the bottom row

### Playing
1. **Position the Gun**: Use left/right buttons to align your gun with the descending invader
2. **Fire**: Press both buttons together to shoot a bullet
3. **Hit the Invader**: Bullet must collide with the invader to destroy it
4. **Avoid Missing**: Each bullet can only hit one invader; missed shots disappear off-screen
5. **Don't Let Them Land**: If an invader reaches the bottom row, game over!

### Scoring
- Each invader destroyed = 1 point
- Score is tracked internally
- At game over, score is displayed as filled rows (max 8)
- Score counter resets after game over

### Difficulty Progression
- Game speed increases with each successful hit
- Invader descent interval decreases: `300ms - (speed × 10ms)`
- Maximum speed level: 20
- At max speed, invaders drop every 100ms

### Game Over Conditions
- **Invader Reaches Bottom**: When an invader reaches row 7 (where the gun is)
- **Visual Feedback**: Display flashes twice
- **Score Display**: Number of filled rows equals score (up to 8 rows)
- **Auto-Restart**: Game automatically resets after 2.5 seconds

## Game States

### Normal Play
- Gun visible at bottom row
- Invader descending from top
- Bullet traveling upward (if fired)
- All elements update in real-time

### Collision (Hit)
1. Bullet and invader occupy same position
2. Flash the hit row 3 times (on/off)
3. Score increments
4. Speed increases
5. New invader spawns at random column
6. Return to normal play

### Game Over
1. Invader reaches bottom row (row 7)
2. Flash entire display twice
3. Display score as filled rows from bottom
4. Wait 2 seconds
5. Clear display
6. Reset all game variables
7. Wait 0.5 seconds
8. Spawn new invader and restart

## Technical Details

### Display Layout
```
Row 0: ████████  ← Invader spawn row
Row 1: ████████
Row 2: ████████
Row 3: ████████
Row 4: ████████
Row 5: ████████
Row 6: ████████  ← Bullet spawn row
Row 7: ████████  ← Gun row (bottom)
       ↑
       Each row has 8 columns (0-7)
```

### Timing Parameters
- **Frame delay**: 1ms per loop iteration
- **Button check interval**: 100ms (debounced)
- **Invader drop interval**: 300ms - (speed × 10ms)
- **Bullet move interval**: 30ms
- **Hit flash duration**: 50ms per flash (6 flashes total)
- **Game over display time**: 2000ms

### Game Variables
- `gunPos`: Current column of gun (0-7), starts at 3
- `invaderCol`: Current column of invader (0-7), random
- `invaderRow`: Current row of invader (0-7), starts at 0
- `bulletActive`: Whether bullet is in flight (true/false)
- `bulletCol`: Current column of bullet (0-7)
- `bulletRow`: Current row of bullet (0-6)
- `score`: Number of hits (0-65535)
- `speed`: Current speed level (1-20)

## Strategy Tips

### Positioning
- Try to position the gun under the invader's column early
- Account for the invader's current position, not where it will be

### Timing
- Fire when the invader is still several rows from the bottom
- Bullets travel faster than invaders, so you have time

### Early Game
- Take your time to line up shots
- Speed is slow initially

### Late Game
- React quickly to new invader positions
- Pre-position for the next invader after each hit
- Speed increases make precision more important

## Code Customization

Users can modify these parameters in the code to adjust gameplay:

### Difficulty
```cpp
// Line 100: Invader drop speed formula
uint16_t invaderDropInterval = 300 - (speed * 10);

// Line 190: Max speed cap
if (speed > 20) speed = 20;
```

### Visual Effects
```cpp
// Line 78: Display brightness (0x00 to 0x0F)
sendCmd(MAX7219_REG_INTENSITY, 0x08);

// Lines 193-198: Hit flash effect duration
delay(50);  // Time between flashes
```

### Controls
```cpp
// Line 101: Bullet speed (lower = faster)
uint16_t bulletMoveInterval = 30;

// Line 102: Button response rate (lower = more responsive)
uint16_t moveInterval = 100;
```

## Differences from Classic Space Invaders

This minimal version differs from the original:
- **Single invader** instead of rows of multiple invaders
- **No shields** or defensive structures
- **No invader shooting** at the player
- **Vertical movement only** for invaders (no side-to-side)
- **Score display** using filled rows instead of digits
- **8x8 display** instead of larger screen
- **Continuous play** instead of discrete levels

## Future Enhancement Ideas

- Multiple simultaneous invaders
- Invader shooting back at gun
- Shield blocks that can be destroyed
- Sound effects using a piezo buzzer
- High score saved in EEPROM
- Boss battles every N points
- Power-ups (rapid fire, shield, etc.)
- Different invader patterns
