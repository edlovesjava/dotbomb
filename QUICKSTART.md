# DotInvaders - Quick Start Guide

Get your game up and running in 5 minutes!

## Prerequisites

- Arduino IDE installed
- LedControl library installed
- Hardware components ready (see PARTS_LIST.md)

## Step-by-Step Setup

### 1. Hardware Assembly (5 minutes)

**Connect MAX7219 to ATTiny85:**
```
MAX7219 VCC  →  ATTiny Pin 8 (VCC)
MAX7219 GND  →  ATTiny Pin 4 (GND)
MAX7219 DIN  →  ATTiny Pin 5 (PB0)
MAX7219 CS   →  ATTiny Pin 6 (PB1)
MAX7219 CLK  →  ATTiny Pin 7 (PB2)
```

**Connect Buttons to ATTiny85:**
```
Left Button:  ATTiny Pin 2 (PB3) → Button → GND
Right Button: ATTiny Pin 3 (PB4) → Button → GND
Fire Button:  ATTiny Pin 1 (PB5) → Button → GND
```

**Power:**
```
5V Power → ATTiny Pin 8 (VCC)
GND      → ATTiny Pin 4 (GND)
```

### 2. Software Setup (2 minutes)

1. Open Arduino IDE
2. Install LedControl library:
   - `Sketch` → `Include Library` → `Manage Libraries`
   - Search "LedControl"
   - Install "LedControl by Eberhard Fahle"

3. For ATTiny85 (skip if using Arduino):
   - `File` → `Preferences`
   - Add to "Additional Boards Manager URLs":
     ```
     https://raw.githubusercontent.com/damellis/attiny/ide-1.6.x-boards-manager/package_damellis_attiny_index.json
     ```
   - `Tools` → `Board` → `Boards Manager`
   - Search "attiny" and install

### 3. Upload the Game (3 minutes)

**Option A: Using Arduino as Programmer**
1. Upload ArduinoISP sketch to your Arduino
2. Connect ATTiny85 to Arduino:
   ```
   ATTiny Pin 1 (RESET) → Arduino Pin 10
   ATTiny Pin 5 (MOSI)  → Arduino Pin 11
   ATTiny Pin 6 (MISO)  → Arduino Pin 12
   ATTiny Pin 7 (SCK)   → Arduino Pin 13
   ATTiny Pin 8 (VCC)   → Arduino 5V
   ATTiny Pin 4 (GND)   → Arduino GND
   ```
3. Arduino IDE settings:
   - `Tools` → `Board` → `ATtiny25/45/85`
   - `Tools` → `Processor` → `ATtiny85`
   - `Tools` → `Clock` → `Internal 8 MHz`
   - `Tools` → `Programmer` → `Arduino as ISP`
4. Click `Sketch` → `Upload Using Programmer`

**Option B: Using Arduino Board Directly**
1. Just upload normally to Arduino Uno/Nano
2. No special programmer needed!

### 4. Test Hardware (Optional but Recommended)

Before uploading the game, test your hardware:

1. Open `hardware_test/hardware_test.ino`
2. Upload to your board
3. Verify:
   - LED matrix lights up and animates
   - All buttons work
   - Display is clear

### 5. Upload and Play!

1. Open `dotinvadors.ino`
2. Upload to your ATTiny85/Arduino
3. Power on
4. Game starts automatically!

## Playing the Game

**Controls:**
- **Left Button** = Move ship left ←
- **Right Button** = Move ship right →
- **Fire Button** = Shoot! 🔥

**Objective:**
- Destroy all alien invaders (top 3 rows)
- Don't let them reach the bottom!
- New wave appears when you clear all enemies

**Game Over:**
- Enemies reach your ship
- Display flashes
- Press Fire to restart

## Troubleshooting Quick Fixes

| Problem | Quick Fix |
|---------|-----------|
| Nothing displays | Check power (5V to VCC, GND to GND) |
| Display scrambled | Check DIN, CLK, CS connections |
| Buttons don't work | Verify buttons connect pin to GND |
| Can't upload | Check programmer connections and settings |
| Game too fast/slow | Modify speed constants in code |

**Still having issues?** See TROUBLESHOOTING.md for detailed help.

## Customization

Want to modify the game? Edit these values in `dotinvadors.ino`:

```cpp
// Make game slower (increase numbers)
#define GAME_SPEED 300      // Was: 200
#define ENEMY_SPEED 700     // Was: 500
#define BULLET_SPEED 150    // Was: 100

// Make display brighter
lc.setIntensity(0, 15);     // Was: 8 (range: 0-15)
```

## Next Steps

- Build a case for your game
- Add sound effects with a piezo buzzer
- Create a custom PCB for permanent installation
- Share your build with friends!

## Support

- **Documentation**: See README.md for full details
- **Hardware Help**: See HARDWARE.md for circuits
- **Parts**: See PARTS_LIST.md for shopping guide
- **Issues**: See TROUBLESHOOTING.md for solutions

---

**Ready to play? Let's go! 👾🚀**

Have fun with DotInvaders!
