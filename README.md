# Dot Invaders

A simple ATTiny85 game with the MAX7219 8x8 LED matrix inspired by Space Invaders.

## Game Description

Dot Invaders is a minimalist space shooter game where:
- An invader dot descends from the top of the screen step by step
- A gun at the bottom row is controlled by two buttons (left and right)
- Pressing both buttons together (chording) fires a bullet upward
- The bullet destroys the invader when they collide
- Score increases with each hit, and the game gets progressively faster
- Game over occurs if an invader reaches the bottom row

## Game Visualization

```
8x8 LED Matrix Display:

┌────────────────┐
│ · · ● · · · · ·│  Row 0 - Invader spawns here
│ · · · · · · · ·│  Row 1
│ · · · · · · · ·│  Row 2
│ · · ○ · · · · ·│  Row 3 - Bullet traveling up
│ · · ▼ · · · · ·│  Row 4 - Invader descending
│ · · · · · · · ·│  Row 5
│ · · · · · · · ·│  Row 6
│ · · ▲ · · · · ·│  Row 7 - Your gun (bottom)
└────────────────┘

Legend:
▲ = Gun (you)
▼ = Invader (enemy)
○ = Bullet (your shot)
● = Target position

Controls:
[LEFT] = Move gun right*
[RIGHT] = Move gun left*
[LEFT + RIGHT] = Fire!

*Button labels reversed due to hardware wiring
```

## Hardware

This game uses the same hardware setup as the [dotris](https://github.com/edlovesjava/dotris) project:

### Components
- **ATtiny85** microcontroller
- **8x8 LED Matrix** with MAX7219 driver
- **2 Push Buttons** (left and right movement)
- **5V Power Supply** (USB or battery)

### Pin Configuration

| ATtiny85 Pin | Arduino Pin | Function | Connected To |
|--------------|-------------|----------|--------------|
| PB0 (Pin 5)  | D0          | DATA     | MAX7219 DIN  |
| PB1 (Pin 6)  | D1          | CLK      | MAX7219 CLK  |
| PB2 (Pin 7)  | D2          | CS       | MAX7219 CS   |
| PB3 (Pin 2)  | D3          | BTN_RIGHT| Right Button |
| PB4 (Pin 3)  | D4          | BTN_LEFT | Left Button  |
| VCC (Pin 8)  | VCC         | Power    | +5V          |
| GND (Pin 4)  | GND         | Ground   | GND          |

## Controls

| Input | Action |
|-------|--------|
| LEFT Button | Move gun right (note: buttons are reversed in hardware) |
| RIGHT Button | Move gun left (note: buttons are reversed in hardware) |
| LEFT + RIGHT | Fire bullet |

## Software Setup

1. Install Arduino IDE
2. Install ATTinyCore board support:
   - Add to Board Manager URLs: `https://drazzy.com/package_drazzy.com_index.json`
   - Install "ATTinyCore" by Spence Konde

3. Configure board settings:
   - Board: "ATtiny25/45/85 (No bootloader)"
   - Chip: "ATtiny85"
   - Clock: "8 MHz (internal)"
   - Programmer: "USBtinyISP" (or your programmer)

4. Connect programmer to ATtiny85
5. Burn bootloader (sets fuses) - one time only
6. Open `ATTiny_DotInvaders/ATTiny_DotInvaders.ino`
7. Upload sketch to ATtiny85

## How to Play

1. Power on the device
2. An invader dot will start descending from a random column
3. Use the left/right buttons to position your gun
4. Press both buttons together to fire a bullet
5. Hit the invader before it reaches the bottom
6. Each successful hit increases your score and game speed
7. If an invader reaches the bottom, the game is over
8. After game over, your score is displayed (number of filled rows)
9. The game automatically resets after a short delay

For detailed testing and troubleshooting, see [TESTING.md](TESTING.md).

## Game Mechanics

- **Invader Movement**: Descends one row at a time with increasing speed
- **Gun Movement**: Positioned at the bottom row, moves left/right
- **Bullet**: Fires from gun position and travels upward rapidly
- **Collision**: Bullet destroys invader on contact
- **Scoring**: Each hit increases score by 1 and speeds up the game
- **Game Over**: Invader reaches row 7 (bottom row with gun)

## License

MIT License - Feel free to modify and share!

## Credits

Hardware setup based on [dotris](https://github.com/edlovesjava/dotris) by edlovesjava.
