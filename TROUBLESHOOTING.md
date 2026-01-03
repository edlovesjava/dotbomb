# DotInvaders - Troubleshooting Guide

## Common Issues and Solutions

### Display Issues

#### Problem: LED Matrix doesn't light up at all

**Possible Causes:**
1. No power to MAX7219 module
2. Wrong wiring connections
3. Faulty MAX7219 module

**Solutions:**
- Verify 5V and GND connections to MAX7219
- Check continuity of all wires
- Measure voltage at MAX7219 VCC pin (should be ~5V)
- Try the hardware test sketch first
- Test MAX7219 with a known-working example sketch

#### Problem: Display shows random patterns or flickers

**Possible Causes:**
1. Loose connections
2. Interference on SPI lines
3. Power supply issues
4. Wrong chip select timing

**Solutions:**
- Ensure all connections are solid (resolder if needed)
- Keep wires short and neat
- Use a stable 5V power supply with adequate current (500mA+)
- Add a 100µF capacitor across VCC and GND near the MAX7219
- Check that CS, CLK, and DIN pins are correctly defined

#### Problem: Some LEDs don't light up

**Possible Causes:**
1. Dead LEDs in the matrix
2. Cold solder joints in the module

**Solutions:**
- Run the hardware test to identify which LEDs are dead
- If multiple LEDs in a row/column are dead, likely a connection issue
- Individual dead LEDs: hardware defect, may need replacement module

### Button Issues

#### Problem: Buttons don't respond

**Possible Causes:**
1. Wrong pin assignments
2. Faulty buttons
3. Missing pull-up resistors (if not using internal)
4. Wrong button wiring

**Solutions:**
- Verify button pins match the code definitions
- Test buttons with a multimeter (should show continuity when pressed)
- Ensure buttons connect pin to GND (not to VCC)
- Check that `INPUT_PULLUP` is set in the code
- Try adding external 10kΩ pull-up resistors

#### Problem: Buttons trigger multiple times (bouncing)

**Possible Causes:**
1. Mechanical button bounce
2. Insufficient debounce delay

**Solutions:**
- The code includes 150ms debounce delays
- Add a 0.1µF capacitor across each button
- Increase debounce delay values in the code if needed

#### Problem: Buttons are reversed

**Possible Causes:**
1. Wiring swapped

**Solutions:**
- Check which physical button corresponds to which pin
- Either rewire or modify pin definitions in code

### Upload Issues

#### Problem: Can't upload to ATTiny85

**Possible Causes:**
1. Programmer not connected correctly
2. Wrong board settings
3. Fuse settings incorrect

**Solutions:**
- Verify programmer connections (check your programmer's pinout)
- In Arduino IDE, ensure:
  - Board: ATtiny25/45/85
  - Processor: ATtiny85
  - Clock: Internal 8 MHz
- Try "Burn Bootloader" to set fuses
- Use slower upload speed if available in programmer settings
- Test programmer with LED blink example first

#### Problem: "Device signature = 0x000000" error

**Possible Causes:**
1. ATTiny85 not powered
2. Poor connection to programmer
3. Wrong programmer selected

**Solutions:**
- Check 5V power to ATTiny85
- Verify all programmer connections
- Select correct programmer in Tools → Programmer menu
- Check if ATTiny85 is inserted correctly (if using IC socket)

### Game Behavior Issues

#### Problem: Game runs too fast or too slow

**Solutions:**
- Adjust timing constants at top of dotinvadors.ino:
  ```cpp
  #define GAME_SPEED 200       // Increase to slow down display updates
  #define ENEMY_SPEED 500      // Increase to slow down enemies
  #define BULLET_SPEED 100     // Increase to slow down bullets
  ```

#### Problem: Enemies don't move correctly

**Solutions:**
- Verify the enemy update logic is working
- Check if the direction change detection is functioning
- This might indicate a code bug - check the updateEnemies() function

#### Problem: Bullets don't detect hits

**Possible Causes:**
1. Collision detection logic error
2. Timing issue

**Solutions:**
- Review the bullet collision code in updateBullet()
- Ensure enemy positions are being tracked correctly
- Add debug LEDs to show collision detection

#### Problem: Game crashes or freezes

**Possible Causes:**
1. Memory overflow (SRAM exhausted)
2. Stack collision
3. Infinite loop

**Solutions:**
- ATTiny85 has very limited RAM (512 bytes)
- Reduce array sizes if you've modified the code
- Check for infinite loops in game logic
- Simplify game if memory is the issue

### Power Issues

#### Problem: Game resets randomly

**Possible Causes:**
1. Insufficient power supply current
2. Power supply voltage drops
3. Brown-out detection triggered

**Solutions:**
- Use a power supply capable of at least 500mA
- Add a large capacitor (100-470µF) across power rails
- Keep wires short to reduce voltage drop
- Check power supply voltage under load (should stay above 4.5V)

#### Problem: LEDs are dim

**Possible Causes:**
1. Power supply current limited
2. Intensity setting too low
3. Voltage drop

**Solutions:**
- Ensure adequate power supply (500mA+)
- Increase intensity in code: `lc.setIntensity(0, 15);` (range 0-15)
- Check voltage at MAX7219 module
- Reduce number of simultaneous lit LEDs

### Development Issues

#### Problem: Can't find LedControl library

**Solutions:**
1. Install via Library Manager:
   - Sketch → Include Library → Manage Libraries
   - Search "LedControl"
   - Install "LedControl by Eberhard Fahle"
2. Or download from: https://github.com/wayoda/LedControl
3. Extract to Arduino libraries folder

#### Problem: Compilation errors

**Common Errors:**
- `'A1' was not declared`: You might need to use pin 6 instead on some boards
- `LOW_POWER not defined`: Not critical, can be ignored for ATTiny
- `Out of memory`: Code too large for ATTiny85, use Arduino Uno for testing

### Testing Procedure

If you're having issues, follow this systematic approach:

1. **Test Power**
   - Measure voltage at all power points
   - Ensure stable 5V supply

2. **Test LED Matrix**
   - Upload a simple MAX7219 example
   - Verify all LEDs work

3. **Test Buttons**
   - Upload hardware_test.ino
   - Verify each button works

4. **Test Game**
   - Upload dotinvadors.ino
   - Verify game mechanics

5. **Optimize**
   - Adjust timing constants
   - Fine-tune gameplay

## Getting Help

If you're still having issues:

1. Run the hardware_test.ino sketch first
2. Document which specific features don't work
3. Check wiring against the circuit diagrams
4. Verify component specifications
5. Try with a full Arduino board first (easier to debug)

## Useful Tools

- **Multimeter**: Test continuity, voltage, and current
- **Logic Analyzer**: Debug SPI communication
- **Oscilloscope**: Verify clock signals
- **USB-Serial Adapter**: Can't use with ATTiny85, but useful with Arduino for debugging

## Additional Resources

- MAX7219 Datasheet
- ATTiny85 Datasheet
- LedControl Library Documentation
- Arduino ATTiny Support Forums

---

Remember: Start simple, test each component individually, then integrate!
