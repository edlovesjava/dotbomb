# Testing Guide for Dot Invaders

This document provides guidance for testing the Dot Invaders game on your ATtiny85 hardware.

## Pre-Upload Verification

Before uploading to the ATtiny85, verify your setup:

### Hardware Checklist
- [ ] ATtiny85 microcontroller properly powered (5V)
- [ ] 8x8 LED Matrix with MAX7219 connected correctly
- [ ] Left button connected to PB4 (Pin 3)
- [ ] Right button connected to PB3 (Pin 2)
- [ ] 0.1µF decoupling capacitor near ATtiny85 VCC/GND
- [ ] All ground connections solid
- [ ] Programmer connected and recognized

### Arduino IDE Setup Checklist
- [ ] ATTinyCore installed
- [ ] Board: "ATtiny25/45/85 (No bootloader)"
- [ ] Chip: "ATtiny85"
- [ ] Clock: "8 MHz (internal)"
- [ ] Programmer: Correct programmer selected
- [ ] Bootloader burned (one-time operation)

## Upload Process

1. Open `ATTiny_DotInvaders/ATTiny_DotInvaders.ino` in Arduino IDE
2. Verify the sketch compiles (Sketch → Verify/Compile)
3. Connect programmer to ATtiny85
4. Upload sketch (Sketch → Upload Using Programmer)
5. Wait for "Done uploading" message

## Post-Upload Testing

### Basic Functionality Test

#### Test 1: Display Initialization
**Expected**: After power-on, display should light up with:
- Gun (single LED) at bottom row, center position
- Invader (single LED) at top row, random column

**If display is blank:**
- Check MAX7219 connections (DIN, CLK, CS)
- Verify 5V power to both ATtiny85 and MAX7219
- Check all ground connections

#### Test 2: Gun Movement
**Steps:**
1. Press LEFT button only
2. Observe gun LED movement
3. Press RIGHT button only  
4. Observe gun LED movement

**Expected:**
- Gun moves horizontally across bottom row
- Gun stops at edges (columns 0 and 7)
- Movement is smooth and responsive

**Note:** Due to hardware wiring, button labels may be reversed (LEFT moves right, RIGHT moves left). This is normal and consistent with dotris.

#### Test 3: Bullet Firing
**Steps:**
1. Position gun under invader
2. Press BOTH buttons together (chord)
3. Release buttons

**Expected:**
- Bullet LED appears one row above gun (row 6)
- Bullet travels upward rapidly
- Only one bullet can exist at a time
- Pressing fire again while bullet is active has no effect

#### Test 4: Collision Detection
**Steps:**
1. Wait for invader to descend partway down screen
2. Position gun under invader column
3. Fire bullet when invader is at mid-screen

**Expected:**
- Bullet travels upward
- When bullet and invader occupy same position:
  - Hit row flashes 3 times (on/off)
  - Invader disappears
  - New invader spawns at top in random column
  - Game continues

#### Test 5: Invader Descent
**Steps:**
1. Power on device
2. Don't press any buttons
3. Watch invader descend

**Expected:**
- Invader starts at top row (row 0)
- Invader moves down one row at a time
- Initial descent is slow (~300ms per row)
- Invader descends smoothly

#### Test 6: Game Over
**Steps:**
1. Power on device
2. Let invader reach bottom row without shooting

**Expected:**
- When invader reaches row 7 (bottom):
  - Entire display flashes twice
  - Score is displayed as filled rows (bottom to top)
  - Display holds for 2 seconds
  - Game resets automatically
  - New invader spawns at top

#### Test 7: Scoring and Speed
**Steps:**
1. Hit multiple invaders in succession
2. Observe game speed changes

**Expected:**
- Each hit increases score by 1
- Invaders descend progressively faster
- Game becomes more challenging
- Speed caps at level 20 (100ms between drops)

## Common Issues and Solutions

### Issue: Display shows random pixels
**Solution:** 
- Check for loose connections
- Add/verify 0.1µF decoupling capacitor
- Ensure stable 5V power supply

### Issue: Buttons don't respond
**Solution:**
- Verify button connections to ground
- Check that buttons are normally-open type
- Test buttons with multimeter
- Verify internal pull-ups are enabled in code (lines 71-72)

### Issue: Game runs too fast/slow
**Solution:**
- Adjust timing constants in code:
  - `invaderDropInterval` (line 100)
  - `bulletMoveInterval` (line 101)
  - `moveInterval` (line 102)
- Re-upload sketch after changes

### Issue: Invader always spawns in same column
**Solution:**
- Random seed may not be working
- Code uses `millis()` at startup for randomness
- Try power cycling device multiple times
- Verify invader spawns at different columns after several restarts

### Issue: Collision detection doesn't work
**Solution:**
- Verify game logic in code
- Check that bullet and invader are in same column
- Watch carefully - collision happens very quickly
- Try slowing down bullet speed for testing

### Issue: Display is too bright/dim
**Solution:**
- Adjust intensity in code line 78:
  ```cpp
  sendCmd(MAX7219_REG_INTENSITY, 0x08);  // 0x00-0x0F
  ```
- 0x00 = dimmest, 0x0F = brightest
- Re-upload after changing

## Memory Usage Verification

The ATtiny85 has limited resources:
- **Flash**: 8KB total
- **SRAM**: 512 bytes
- **EEPROM**: 512 bytes

This sketch should use approximately:
- **Flash**: ~2-3 KB
- **SRAM**: ~50-100 bytes

If you encounter memory issues:
- Check compiler output for memory usage
- Reduce flash animation durations
- Remove unnecessary comments
- Optimize variable types

## Advanced Testing

### Stress Test
1. Play continuously for 10+ minutes
2. Verify no crashes or freezes
3. Check for memory leaks (game should run indefinitely)

### Edge Cases
1. **Rapid button pressing**: Mash both buttons quickly
2. **Holding buttons**: Hold fire buttons continuously
3. **Simultaneous fire and move**: Press all buttons in various combinations
4. **Maximum score**: Play until speed = 20, verify game remains stable

### Timing Verification
1. Measure invader drop rate with stopwatch
2. Initial: ~300ms per row
3. After 10 hits: ~200ms per row (300 - 10*10)
4. At max speed (20): ~100ms per row (300 - 20*10)

## Expected Behavior Summary

| Condition | Expected Result |
|-----------|----------------|
| Power on | Gun at center bottom, invader at random top |
| Press LEFT | Gun moves (right or left depending on wiring) |
| Press RIGHT | Gun moves (opposite of LEFT) |
| Press BOTH | Bullet fires from gun position |
| Bullet hits invader | Flash effect, new invader, score++ |
| Invader reaches bottom | Game over sequence, score display, reset |
| Multiple hits | Increasing speed, max at level 20 |
| Bullet misses | Bullet disappears at top, no effect |

## Debugging Tips

### Enable Serial Debug (if using Arduino Uno as ISP)
If programming via Arduino Uno as ISP, you could temporarily add serial debugging:
```cpp
// In setup():
Serial.begin(9600);

// In loop():
Serial.print("Invader: ");
Serial.print(invaderCol);
Serial.print(",");
Serial.println(invaderRow);
```

Note: This won't work on final ATtiny85 deployment (no serial pins).

### Visual Debugging
Add longer delays to slow down game for observation:
```cpp
delay(500);  // Slow down game loop temporarily
```

### Test on Arduino Uno First
Before deploying to ATtiny85, test on Arduino Uno with same pin assignments to verify logic.

## Success Criteria

Your implementation is successful if:
- ✅ Display initializes correctly on power-up
- ✅ Gun moves smoothly left and right
- ✅ Bullets fire when both buttons are pressed together
- ✅ Collisions are detected accurately
- ✅ Invaders descend at appropriate speed
- ✅ Game over sequence works correctly
- ✅ Game resets and continues indefinitely
- ✅ Score tracking and speed progression work
- ✅ No crashes or freezes during extended play

## Reporting Issues

If you encounter persistent issues:
1. Document the exact behavior you observe
2. Note your hardware configuration
3. Check Arduino IDE compiler output for errors
4. Verify all connections with multimeter
5. Test with a known-good ATtiny85 if available
6. Open an issue on the GitHub repository with details
