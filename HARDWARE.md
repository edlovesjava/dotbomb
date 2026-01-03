# Circuit Diagram and Technical Details

## Simplified Circuit Diagram

```
                           ATTiny85
                         ┌─────────┐
                         │1      8 │──── VCC (5V)
    Button FIRE ────┤    │         │
                         │2      7 │──── CLK (to MAX7219)
    Button LEFT ────┤    │         │
                         │3      6 │──── CS (to MAX7219)
    Button RIGHT ───┤    │         │
                         │4      5 │──── DIN (to MAX7219)
          GND ───────────│         │
                         └─────────┘
                         

                         MAX7219 LED Matrix Module
                         ┌────────────────┐
        VCC (5V) ────────│ VCC        8x8 │
        GND ─────────────│ GND        LED │
        DIN (PB0) ───────│ DIN      MATRIX│
        CS (PB1) ────────│ CS             │
        CLK (PB2) ───────│ CLK            │
                         └────────────────┘
```

## Pin Configuration Details

### ATTiny85 Pin Mapping

| Physical Pin | ATTiny85 Name | Function      | Connection         |
|--------------|---------------|---------------|--------------------|
| Pin 1        | PB5 (RESET)   | Fire Button   | Button to GND      |
| Pin 2        | PB3           | Left Button   | Button to GND      |
| Pin 3        | PB4           | Right Button  | Button to GND      |
| Pin 4        | GND           | Ground        | GND rail           |
| Pin 5        | PB0 (MOSI)    | DIN           | MAX7219 DIN        |
| Pin 6        | PB1           | CS            | MAX7219 CS         |
| Pin 7        | PB2 (SCK)     | CLK           | MAX7219 CLK        |
| Pin 8        | VCC           | Power         | +5V rail           |

### Button Connections

Each button should be wired as follows:
```
ATTiny Pin ────┤ │──── GND
              Button
```

The internal pull-up resistors are enabled in the code, so external pull-up resistors are optional but recommended for better noise immunity.

## Power Considerations

- **ATTiny85**: Typical consumption ~1-8mA (depending on clock speed)
- **MAX7219 + LEDs**: Up to 330mA (when all LEDs are on at max brightness)
- **Recommended power supply**: 5V, 500mA minimum

### Power Supply Options

1. **USB Power**: 5V from USB port (500mA available)
2. **Battery**: 3x AA batteries with voltage regulator to 5V
3. **Wall Adapter**: 5V regulated DC adapter

## SPI Communication

The MAX7219 uses SPI-like communication:
- **DIN (Data In)**: Serial data input
- **CLK (Clock)**: Clock signal for data synchronization
- **CS (Chip Select)**: Activates the device

The LedControl library handles the SPI communication protocol automatically.

## Memory Usage Estimates

ATTiny85 has limited resources:
- **Flash Memory**: 8KB (program storage)
- **SRAM**: 512 bytes (runtime variables)
- **EEPROM**: 512 bytes (persistent storage, unused in this project)

This program is optimized to fit within ATTiny85 constraints:
- Estimated Flash usage: ~4-5KB
- Estimated SRAM usage: ~200 bytes

## Alternative Hardware

This code can also run on:
- **Arduino Uno/Nano**: Full Arduino boards (easier development)
- **ATTiny84**: More pins available
- **ATmega328**: Standard Arduino chip

Just adjust pin numbers in the code for different boards.

## LED Matrix Details

The MAX7219 controls an 8x8 LED matrix:
- **Total LEDs**: 64 individual LEDs
- **Addressing**: Row (0-7) and Column (0-7)
- **Brightness**: 16 intensity levels
- **Scan Rate**: Multiplexed (one row at a time, very fast)

### Coordinate System

```
    0 1 2 3 4 5 6 7  (Column X)
  ┌─────────────────┐
0 │ . . . . . . . . │
1 │ . . . . . . . . │
2 │ . . . . . . . . │
3 │ . . . . . . . . │
4 │ . . . . . . . . │
5 │ . . . . . . . . │
6 │ . . . . . . . . │
7 │ . . . . . . . . │
  └─────────────────┘
(Row Y)
```

- **Player**: Row 7 (bottom)
- **Enemies**: Rows 0-2 (top)
- **Bullets**: Move from row 7 upward to row 0

## Debugging Tips

1. **Test MAX7219 First**: Use a simple test sketch to verify the LED matrix works
2. **Serial Debug**: Not available on ATTiny85 (use Arduino for development)
3. **LED Indicators**: Use the matrix itself to display debug info
4. **Scope/Logic Analyzer**: Check SPI signals if communication fails

## Performance Optimization

The code uses several optimization techniques:
- **Timing Control**: Separate update rates for different game elements
- **Minimal Redraw**: Only updates display when needed
- **Boolean Arrays**: Efficient enemy tracking
- **Simple Math**: Avoids floating-point operations

## Future Enhancements

Possible improvements (require more memory/pins):
- Enemy shooting
- Multiple bullet support
- Sound effects (piezo buzzer)
- Score display on separate 7-segment display
- High score storage in EEPROM
- Power-saving sleep mode
- Multiple enemy patterns
- Boss battles
