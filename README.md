# DotInvaders 👾

A simplified Space Invaders game for ATTiny85 (or Arduino) with MAX7219 8x8 LED Matrix.

![Game Type](https://img.shields.io/badge/Game-Space%20Invaders-blue)
![Platform](https://img.shields.io/badge/Platform-ATTiny85%20%7C%20Arduino-green)
![License](https://img.shields.io/badge/License-MIT-yellow)

## Quick Links

- 🚀 **[Quick Start Guide](QUICKSTART.md)** - Get playing in 5 minutes!
- 🔧 **[Hardware Details](HARDWARE.md)** - Circuit diagrams and specs
- 🛒 **[Parts List](PARTS_LIST.md)** - What to buy and where
- 🔍 **[Troubleshooting](TROUBLESHOOTING.md)** - Fix common issues

## Overview

DotInvaders is a minimalist version of the classic Space Invaders arcade game, designed to run on an 8x8 LED matrix. Control your ship at the bottom of the screen, shoot at the invading aliens, and try to destroy them all before they reach you!

## Hardware Requirements

- **Microcontroller**: ATTiny85 or any Arduino-compatible board
- **Display**: MAX7219 8x8 LED Matrix Module
- **Controls**: 3 Push Buttons (Left, Right, Fire)
- **Power**: 5V power supply
- **Resistors**: 10kΩ pull-up resistors for buttons (if not using internal pull-ups)

## Wiring Diagram

### MAX7219 LED Matrix to ATTiny85

| MAX7219 Pin | ATTiny85 Pin | Arduino Pin |
|-------------|--------------|-------------|
| VCC         | Pin 8 (VCC)  | 5V          |
| GND         | Pin 4 (GND)  | GND         |
| DIN         | Pin 5 (PB0)  | Pin 0       |
| CS          | Pin 6 (PB1)  | Pin 1       |
| CLK         | Pin 7 (PB2)  | Pin 2       |

### Control Buttons

| Button | ATTiny85 Pin | Arduino Pin |
|--------|--------------|-------------|
| Left   | Pin 2 (PB3)  | Pin 3       |
| Right  | Pin 3 (PB4)  | Pin 4       |
| Fire   | Pin 1 (PB5)  | A1 (Pin 6)  |

**Note**: All buttons should be connected between the pin and GND. The code uses internal pull-up resistors.

## Software Requirements

### Arduino IDE Setup

1. Install Arduino IDE (1.8.x or newer)
2. Install the **LedControl** library:
   - Open Arduino IDE
   - Go to `Sketch` → `Include Library` → `Manage Libraries`
   - Search for "LedControl"
   - Install "LedControl by Eberhard Fahle"

### ATTiny85 Support (if using ATTiny85)

1. Add ATTiny board support:
   - Go to `File` → `Preferences`
   - Add this URL to "Additional Board Manager URLs":
     ```
     https://raw.githubusercontent.com/damellis/attiny/ide-1.6.x-boards-manager/package_damellis_attiny_index.json
     ```
   - Go to `Tools` → `Board` → `Boards Manager`
   - Search for "attiny" and install the package

2. Configure the board:
   - Select `Tools` → `Board` → `ATtiny25/45/85`
   - Select `Tools` → `Processor` → `ATtiny85`
   - Select `Tools` → `Clock` → `Internal 8 MHz`

## Installation

1. Download or clone this repository
2. Open `dotinvadors.ino` in Arduino IDE
3. Connect your ATTiny85 programmer or Arduino board
4. Upload the sketch

## How to Play

### Game Controls

- **Left Button**: Move your ship left
- **Right Button**: Move your ship right
- **Fire Button**: Shoot a bullet upward

### Gameplay

1. Your ship is represented by a single LED at the bottom of the screen
2. Enemy invaders appear as LEDs in the top rows
3. The enemies move left and right, gradually descending
4. Shoot the enemies before they reach the bottom
5. If all enemies are destroyed, a new wave appears
6. Game over when enemies reach your position

### Display Elements

- **Bottom Row**: Your ship (one LED)
- **Top 3 Rows**: Enemy formation (multiple LEDs)
- **Moving Dot**: Your bullet when fired

### Game Layout on 8x8 Matrix

```
Row 0:  👾 . 👾 . 👾 . 👾 .    (Enemy Row 1)
Row 1:  👾 . 👾 . 👾 . 👾 .    (Enemy Row 2)
Row 2:  👾 . 👾 . 👾 . 👾 .    (Enemy Row 3)
Row 3:  . . . . . . . .        (Empty)
Row 4:  . . . . . . . .        (Empty)
Row 5:  . . . . * . . .        (Bullet)
Row 6:  . . . . . . . .        (Empty)
Row 7:  . . . 🚀 . . . .       (Your Ship)
```

Each LED represents:
- 🚀 = Player ship (you)
- 👾 = Enemy invader
- * = Bullet (when fired)

## Game Features

- ✨ Smooth enemy movement patterns
- 🎯 Collision detection for bullets and enemies
- 📊 Score tracking (counts destroyed enemies)
- 🎮 Simple 3-button control scheme
- 🔄 Progressive difficulty (new wave after clearing)
- 💀 Game over detection

## Customization

You can modify these constants in the code to adjust gameplay:

```cpp
#define GAME_SPEED 200       // Display update rate (ms)
#define ENEMY_SPEED 500      // Enemy movement speed (ms)
#define BULLET_SPEED 100     // Bullet movement speed (ms)
```

## Troubleshooting

### Display not working
- Check all wiring connections
- Verify MAX7219 has proper 5V power
- Test MAX7219 with a simple example sketch

### Buttons not responsive
- Verify buttons are connected to correct pins
- Check if internal pull-ups are enabled
- Try adding external 10kΩ pull-up resistors

### ATTiny85 upload issues
- Ensure correct board and processor selected
- Check programmer connections
- Try slower upload speed

## License

This project is open source and available for educational and personal use.

## Credits

Inspired by the classic Space Invaders arcade game and designed to demonstrate simple game mechanics on minimal hardware.

---

**Enjoy playing DotInvaders! 👾🚀**
