# Dot Invaders Hardware Specification

## Overview

Ultra-low-power retro handheld game platform featuring:
- 8x8 LED matrix display
- Two-button input (with optional third button)
- Battery or USB powered
- In-circuit USB programming
- 3D printable enclosure

---

## Platform Goals

| Goal | Target |
|------|--------|
| Power consumption | <20mA active, <1µA sleep |
| Battery life | 20+ hours on 2xAAA or coin cell |
| Cost | <$10 BOM |
| Size | Pocket-sized (~60x40x15mm) |
| Programming | USB in-place, no disassembly |

---

## Core Components

### Microcontroller: ATtiny85

| Spec | Value |
|------|-------|
| Package | 8-pin DIP or SOIC |
| Flash | 8KB |
| SRAM | 512 bytes |
| EEPROM | 512 bytes |
| Clock | 8MHz internal (16MHz PLL available) |
| Operating voltage | 2.7-5.5V |
| Active current | ~1mA @ 1MHz, ~5mA @ 8MHz |
| Sleep current | <0.1µA power-down mode |

**Pin Assignment:**
```
ATtiny85 Pinout:
                 ┌──────┐
    (RESET) PB5 ─┤1    8├─ VCC
            PB3 ─┤2    7├─ PB2 (SCK/USB D+)
            PB4 ─┤3    6├─ PB1 (MISO/USB D-)
            GND ─┤4    5├─ PB0 (MOSI)
                 └──────┘

Current Assignment:
  PB0 - MAX7219 DIN (data)
  PB1 - MAX7219 CLK (clock) / USB D-
  PB2 - MAX7219 CS (chip select) / USB D+
  PB3 - RIGHT button (active low, internal pullup)
  PB4 - LEFT button (active low, internal pullup)
  PB5 - RESET (or third button if fuse disabled)
```

### Display: MAX7219 + 8x8 LED Matrix

| Spec | Value |
|------|-------|
| Driver IC | MAX7219 |
| Display | 8x8 common cathode LED matrix |
| Interface | 3-wire SPI (DIN, CLK, CS) |
| Current per LED | 40mA max (adjustable via ISET resistor) |
| Scan rate | 800Hz (no flicker) |
| Operating voltage | 4.0-5.5V |

**Power Considerations:**
- MAX7219 intensity register (0x0A) controls brightness 0-15
- At intensity 8: ~100mA with all LEDs on
- At intensity 2: ~30mA with all LEDs on
- Typical gameplay: ~20-50mA

### Buttons

| Button | Function | Pin |
|--------|----------|-----|
| LEFT | Move left / Fire (chord) | PB4 |
| RIGHT | Move right / Fire (chord) | PB3 |
| RESET* | Reset / Third button | PB5 |

*Optional: PB5 can be used as third button if RSTDISBL fuse is programmed (loses ISP capability, requires HV programming)

**Button Specs:**
- Type: Tactile momentary switch
- Travel: 0.25mm typical
- Force: 160-260gf
- Debounce: Software (handled in code)

---

## Power System

### Power Options

#### Option A: USB Only (5V)
```
USB 5V ──┬── VCC (ATtiny85)
         └── VCC (MAX7219)
```
- Simplest design
- Requires USB connection to play
- No battery management needed

#### Option B: 2xAAA Battery
```
2xAAA (3V) ──[Boost Converter]── 5V ── VCC
```
- Boost converter (e.g., MCP1640, TPS61220)
- ~85% efficiency
- 20+ hours playtime

#### Option C: USB + Battery (Recommended)
```
USB 5V ──[Schottky Diode]──┬── VCC
                           │
LiPo 3.7V ─[Boost]─────────┘
          ─[Charger TP4056]── USB 5V
```
- Best of both worlds
- Play while charging
- LiPo or 2xAAA options

### Power Budget

| Component | Active (mA) | Sleep (µA) |
|-----------|-------------|------------|
| ATtiny85 @ 8MHz | 5 | 0.1 |
| MAX7219 (intensity 8) | 50 | 10 |
| Boost converter | 2 | 10 |
| **Total** | **~57** | **~20** |

With 2xAAA (2000mAh): ~35 hours active play

---

## USB Programming Interface

### Micronucleus Bootloader

The ATtiny85 can be programmed via USB using the Micronucleus bootloader:

| Spec | Value |
|------|-------|
| Bootloader size | ~1.5KB |
| Available flash | ~6.5KB |
| USB protocol | V-USB (software USB 1.1) |
| Programming | avrdude with micronucleus protocol |

**USB Connection:**
```
USB D- ── 68Ω ── PB1
USB D+ ── 68Ω ── PB2
         ├── 1.5kΩ pullup to VCC (via 3.6V zener)
USB GND ── GND
USB 5V ── VCC
```

**Note:** USB D+ needs 1.5kΩ pullup to 3.3-3.6V for device detection.

### Pin Conflict Resolution

PB1 and PB2 are shared between MAX7219 and USB:

| Mode | PB1 | PB2 |
|------|-----|-----|
| Game running | MAX7219 CLK | MAX7219 CS |
| USB programming | USB D- | USB D+ |

**Solutions:**
1. **Physical switch** - DPDT switch to select mode
2. **Jumper** - Solder jumper or header pins
3. **Auto-detect** - Bootloader checks USB connection at startup
4. **Dedicated USB port** - Use programming header only

**Recommended:** Option 3 (auto-detect) - Micronucleus checks for USB host at reset. If found, enters bootloader. Otherwise boots game.

---

## Schematic

```
                                    VCC (+5V)
                                      │
                     ┌────────────────┼────────────────┐
                     │                │                │
                    ┌┴┐              ┌┴┐            ┌──┴──┐
                    │ │10k           │ │10k         │     │
                    │ │              │ │            │ 8x8 │
                    └┬┘              └┬┘            │ LED │
    ┌────────┐       │                │             │     │
    │        │1      8                │             └──┬──┘
    │ PB5/RST├───────┼───VCC         │                │
    │        │       │                │             ┌──┴──┐
    │    PB3 ├───────┴──[BTN]──GND   │             │     │
    │        │                        │             │MAX  │
    │    PB4 ├─────────[BTN]──GND    │             │7219 │
    │        │                        │             │     │
    │    GND ├─────────GND           │             └──┬──┘
    │        │                        │                │
    │    PB0 ├────────────────────────┼──────DIN──────┘
    │        │                        │                │
    │    PB1 ├───────[68Ω]───USB_D-  ├──────CLK──────┘
    │        │                        │                │
    │    PB2 ├───────[68Ω]───USB_D+ ─┼──────CS───────┘
    │        │                 │      │
    └────────┘                ┌┴┐     │
     ATtiny85                 │ │1.5k │
                              └┬┘     │
                               │      │
                              [Z]3.6V │
                               │      │
                              GND    GND

USB Connector:
  Pin 1: VCC (+5V)
  Pin 2: D- (via 68Ω to PB1)
  Pin 3: D+ (via 68Ω to PB2, 1.5k pullup via 3.6V zener)
  Pin 4: GND
```

---

## PCB Design

### Board Specifications

| Spec | Value |
|------|-------|
| Dimensions | 50 x 35 mm (fits 8x8 matrix) |
| Layers | 2 (top + bottom) |
| Thickness | 1.6mm |
| Copper | 1oz |
| Finish | HASL or ENIG |
| Solder mask | Matte black |
| Silkscreen | White |

### Component Placement

```
Top View:
┌─────────────────────────────────┐
│  ┌───────────────────────────┐  │
│  │                           │  │
│  │       8x8 LED MATRIX      │  │
│  │                           │  │
│  │                           │  │
│  └───────────────────────────┘  │
│                                 │
│  [USB]     [ATtiny85]    [MAX]  │
│            (socket)      7219   │
│                                 │
│    [LEFT BTN]    [RIGHT BTN]    │
│                                 │
│  [BAT+]                  [BAT-] │
└─────────────────────────────────┘

Bottom View:
- SMD passives (resistors, caps)
- Battery contacts or JST connector
```

### Design Files Needed

- [ ] Schematic (KiCad or EasyEDA)
- [ ] PCB layout
- [ ] BOM (Bill of Materials)
- [ ] Gerber files for fabrication
- [ ] Pick-and-place file (if SMD assembly)

---

## 3D Printable Enclosure

### Enclosure Requirements

| Requirement | Specification |
|-------------|---------------|
| Material | PLA or PETG |
| Wall thickness | 1.5-2mm |
| Print orientation | Split horizontal |
| Button access | Cutouts for tactile switches |
| USB access | Side cutout for programming |
| Battery access | Removable back or door |
| Viewing angle | 0° (straight on) |

### Dimensions (Estimated)

```
External: 60 x 45 x 18mm
Internal: 56 x 41 x 14mm

Side View:
┌─────────────────────┐
│   ╔═══════════════╗ │  <- LED matrix visible
│   ║               ║ │
│   ║   DISPLAY     ║ │
│   ║               ║ │
│   ╚═══════════════╝ │
│                     │
│  [BTN]       [BTN]  │  <- Button caps protruding
│                     │
├─────────────────────┤
│      PCB HERE       │
├─────────────────────┤
│   BATTERY DOOR      │
└─────────────────────┘
       ↑
    USB port cutout
```

### Design Files Needed

- [ ] Top shell (STL)
- [ ] Bottom shell (STL)
- [ ] Button caps (STL)
- [ ] Battery door (STL) - optional
- [ ] Assembly instructions

---

## Bill of Materials (BOM)

| Qty | Component | Value | Package | Est. Cost |
|-----|-----------|-------|---------|-----------|
| 1 | ATtiny85 | - | DIP-8 | $1.50 |
| 1 | MAX7219 | - | DIP-24 | $1.00 |
| 1 | 8x8 LED Matrix | Common cathode | 20x20mm | $1.00 |
| 1 | DIP-8 socket | - | - | $0.10 |
| 1 | DIP-24 socket | - | - | $0.20 |
| 2 | Tactile switch | 6x6mm | THT | $0.10 |
| 1 | Micro USB connector | - | SMD | $0.30 |
| 2 | Resistor | 68Ω | 0603 | $0.02 |
| 1 | Resistor | 1.5kΩ | 0603 | $0.01 |
| 1 | Resistor | 10kΩ | 0603 | $0.01 |
| 1 | Resistor | 27kΩ | 0603 | $0.01 |
| 1 | Zener diode | 3.6V | SOD-123 | $0.05 |
| 2 | Capacitor | 100nF | 0603 | $0.02 |
| 1 | Capacitor | 10µF | 0805 | $0.05 |
| 1 | 2xAAA holder | - | - | $0.50 |
| | | | **Total** | **~$5.00** |

*Optional additions:*
- LiPo battery + TP4056 charger: +$3
- On/off switch: +$0.20
- 3D printed case: +$1 (material)

---

## Development Phases

### Phase 1: Breadboard Prototype
- [ ] Wire up ATtiny85 + MAX7219 + matrix on breadboard
- [ ] Test game code
- [ ] Verify USB programming with Micronucleus
- [ ] Measure power consumption

### Phase 2: PCB Design
- [ ] Create schematic in KiCad/EasyEDA
- [ ] Design PCB layout
- [ ] Generate Gerbers
- [ ] Order prototype PCBs (JLCPCB, PCBWay)

### Phase 3: Enclosure Design
- [ ] Design enclosure in Fusion 360/OpenSCAD
- [ ] 3D print prototype
- [ ] Iterate on fit and feel
- [ ] Finalize STL files

### Phase 4: Assembly & Testing
- [ ] Assemble first complete unit
- [ ] Test all functions
- [ ] Battery life test
- [ ] Document assembly process

---

## Open Questions

1. **Third button?** Use PB5 (loses ISP) or add shift register?
2. **Battery type?** 2xAAA, CR2032 coin cell, or LiPo?
3. **USB connector?** Micro-USB, USB-C, or bare pads?
4. **Speaker?** Add piezo for audio feedback? (uses 1 pin)
5. **LED matrix style?** Round dots, square pixels, or diffused?

---

## References

- [Micronucleus Bootloader](https://github.com/micronucleus/micronucleus)
- [V-USB Library](https://www.obdev.at/products/vusb/)
- [ATtiny85 Datasheet](https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-2586-AVR-8-bit-Microcontroller-ATtiny25-ATtiny45-ATtiny85_Datasheet.pdf)
- [MAX7219 Datasheet](https://datasheets.maximintegrated.com/en/ds/MAX7219-MAX7221.pdf)
