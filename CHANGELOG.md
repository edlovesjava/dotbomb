# Changelog

All notable changes to DotInvaders will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2026-01-03

### Added - Initial Release

#### Game Features
- Complete Space Invaders game mechanics for 8x8 LED matrix
- Player ship control (left/right movement at bottom row)
- Enemy formation (3 rows with alternating pattern)
- Single bullet shooting mechanics
- Collision detection between bullets and enemies
- Enemy AI with horizontal movement and descent
- Game state management (start, playing, game over)
- Score tracking system
- Lives tracking (3 lives)
- Win condition (clear all enemies, new wave appears)
- Game over detection (enemies reach player)
- Start and game over animations

#### Hardware Support
- ATTiny85 microcontroller support
- Arduino (Uno/Nano) compatibility
- MAX7219 8x8 LED matrix driver integration
- 3-button input system (left, right, fire)
- Internal pull-up resistors for buttons
- Optimized for ATTiny85 constraints (8KB flash, 512B RAM)

#### Code Structure
- Main game file: `dotinvadors.ino` (340 lines)
- Hardware test utility: `hardware_test/hardware_test.ino` (166 lines)
- Clean, modular function organization
- Configurable timing constants
- Efficient memory usage

#### Documentation
- **README.md**: Complete project overview and setup guide
- **QUICKSTART.md**: 5-minute setup guide for beginners
- **HARDWARE.md**: Detailed circuit diagrams and technical specs
- **TROUBLESHOOTING.md**: Comprehensive troubleshooting guide
- **PARTS_LIST.md**: Shopping guide with pricing and sources
- **LICENSE**: MIT license
- **CHANGELOG.md**: This file

#### Technical Specifications
- Display refresh rate: Configurable (default 200ms)
- Enemy movement speed: Configurable (default 500ms)
- Bullet movement speed: Configurable (default 100ms)
- Memory footprint: ~200 bytes RAM, ~4-5KB flash
- Enemy count: 12 enemies (4 per row × 3 rows)
- Game grid: 8×8 pixels

#### Pin Assignments
- MAX7219 DIN: Pin 0 (MOSI)
- MAX7219 CLK: Pin 2 (SCK)
- MAX7219 CS: Pin 1
- Button Left: Pin 3
- Button Right: Pin 4
- Button Fire: Pin A1 (Pin 6 on ATTiny85)

### Dependencies
- LedControl library by Eberhard Fahle (for MAX7219 control)
- Arduino IDE 1.8.x or newer
- ATTiny board support (optional, for ATTiny85)

### Known Limitations
- Single bullet limit (only one bullet can be active at a time)
- No enemy shooting (to conserve memory)
- No sound effects (no piezo buzzer support yet)
- No persistent high score storage
- Fixed enemy pattern (no randomization)
- No difficulty progression (same speed each wave)

### Future Enhancements (Planned)
- Multiple bullet support
- Enemy shooting mechanics
- Sound effects with piezo buzzer
- EEPROM high score storage
- Variable difficulty levels
- Multiple enemy patterns
- Power-saving sleep mode
- Boss battles
- Different enemy movement patterns

---

## Version History

### [1.0.0] - 2026-01-03
- Initial release of DotInvaders
- Complete playable game
- Full documentation suite
- Hardware test utility

---

**Note**: Version numbers follow Semantic Versioning (MAJOR.MINOR.PATCH)
- MAJOR: Incompatible changes
- MINOR: New features (backwards compatible)
- PATCH: Bug fixes (backwards compatible)
