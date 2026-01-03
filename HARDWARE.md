# Hardware Setup for Dot Invaders

## Components Required

- **ATtiny85** microcontroller (8-bit AVR)
- **8x8 LED Matrix** with MAX7219 driver
- **2x Push Buttons** (normally open)
- **5V Power Supply** (USB or battery, ~200mA)
- **0.1µF capacitor** - power supply decoupling
- **Breadboard and jumper wires** (or custom PCB)

## Circuit Connections

### ATtiny85 Pinout
```
        ┌─────┐
 RESET ─┤1   8├─ VCC (+5V)
  PB3  ─┤2   7├─ PB2 (CLK)
  PB4  ─┤3   6├─ PB1 (DIN)
  GND  ─┤4   5├─ PB0 (CS)
        └─────┘
```

### Connection Table

| ATtiny85 Pin | Arduino Pin | Function | Connected To |
|--------------|-------------|----------|--------------|
| PB0 (Pin 5)  | D0          | CS       | MAX7219 CS   |
| PB1 (Pin 6)  | D1          | DIN      | MAX7219 DIN  |
| PB2 (Pin 7)  | D2          | CLK      | MAX7219 CLK  |
| PB3 (Pin 2)  | D3          | BTN_RIGHT| Right Button |
| PB4 (Pin 3)  | D4          | BTN_LEFT | Left Button  |
| VCC (Pin 8)  | VCC         | Power    | +5V          |
| GND (Pin 4)  | GND         | Ground   | GND          |

### MAX7219 LED Matrix Connection
```
ATtiny85                    MAX7219
  PB0 (D0) ──────────────> CS/LOAD
  PB1 (D1) ──────────────> DIN
  PB2 (D2) ──────────────> CLK
  VCC      ──────────────> VCC
  GND      ──────────────> GND
```

### Button Connections
```
Each button (Right and Left):
  One terminal ──> ATtiny85 Pin (PB3 for RIGHT, PB4 for LEFT)
  Other terminal ──> GND

Note: Internal pull-ups are enabled in software, so no external resistors needed
```

### Power Supply
```
  +5V ──┬──> ATtiny85 VCC (Pin 8)
        ├──> MAX7219 VCC
        └──> 0.1µF capacitor to GND
        
  GND ──┬──> ATtiny85 GND (Pin 4)
        ├──> MAX7219 GND
        ├──> All buttons (common)
        └──> 0.1µF capacitor to +5V
```

## Assembly Notes

1. **Decoupling Capacitor**: Place the 0.1µF capacitor as close as possible to ATtiny85 VCC and GND pins to filter power supply noise.

2. **Button Wiring**: Connect buttons between the ATtiny85 pins and ground. The internal pull-up resistors are enabled in the code.

3. **Power Supply**: Ensure your power supply can provide at least 200mA for the LED matrix. USB power is typically sufficient.

4. **LED Matrix Orientation**: The MAX7219 module may need to be oriented correctly. Test with the display to ensure rows and columns are aligned as expected.

## Programming the ATtiny85

### Requirements
- **USBtinyISP** or similar AVR programmer
- **Arduino IDE** with ATTinyCore support

### Setup Steps

1. **Install ATTinyCore** in Arduino IDE:
   - Open Arduino IDE
   - Go to File → Preferences
   - Add to "Additional Board Manager URLs": 
     ```
     https://drazzy.com/package_drazzy.com_index.json
     ```
   - Go to Tools → Board → Boards Manager
   - Search for "ATTinyCore" and install it

2. **Configure Board Settings**:
   - Board: "ATtiny25/45/85 (No bootloader)"
   - Chip: "ATtiny85"
   - Clock: "8 MHz (internal)"
   - Programmer: "USBtinyISP" (or your specific programmer)

3. **Connect Programmer**:
   - Connect USBtinyISP (or your programmer) to ATtiny85
   - Typical programming connections:
     - MOSI → PB0
     - MISO → PB1
     - SCK → PB2
     - RESET → Pin 1
     - VCC → Pin 8
     - GND → Pin 4

4. **Burn Bootloader** (one time only):
   - Tools → Burn Bootloader
   - This sets the fuses for 8MHz internal clock

5. **Upload Sketch**:
   - Open `ATTiny_DotInvaders.ino`
   - Click Upload (or Sketch → Upload Using Programmer)

### Fuse Settings
- Low Fuse: 0xE2 (8MHz internal oscillator)
- High Fuse: 0xDF (default)
- Extended Fuse: 0xFF (default)

## Troubleshooting

### Display not working
- Verify all MAX7219 connections (DIN, CLK, CS)
- Check that both ATtiny85 and MAX7219 are receiving 5V
- Verify ground connections are solid
- Try adjusting MAX7219 intensity in code (line 78)

### Buttons not responding
- Check button connections to ground
- Verify buttons are normally-open type
- Test buttons with a multimeter in continuity mode
- Ensure internal pull-ups are enabled in code (lines 71-72)

### Game runs too fast/slow
- Adjust timing intervals in the code:
  - `invaderDropInterval` (line 100) - controls invader speed
  - `bulletMoveInterval` (line 101) - controls bullet speed
  - `moveInterval` (line 102) - controls button response

### Code won't upload
- Verify programmer connections to ATtiny85
- Check that correct board and chip are selected in Arduino IDE
- Try burning bootloader again
- Ensure programmer is recognized by your computer

### Random behavior or crashes
- Add or check the decoupling capacitor (0.1µF)
- Verify power supply is stable and adequate
- Check for loose connections on breadboard

## Power Consumption

- **Typical**: ~200mA (LED matrix + ATtiny85)
- **Maximum**: ~400mA (all LEDs on at full brightness)
- **Standby**: ~5mA (display off)

## Notes

- This hardware setup is identical to the [dotris](https://github.com/edlovesjava/dotris) project
- The MAX7219 uses bit-banged SPI (software SPI) on the ATtiny85
- No additional external components required beyond those listed
- Can be powered by USB (5V) or 3× AA batteries with a boost converter
