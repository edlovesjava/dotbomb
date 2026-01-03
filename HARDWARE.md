# Hardware Setup

DOTinvaders runs on the **DOTplatform** handheld gaming system.

For complete hardware documentation, see:
**[DOTplatform Hardware Specification](https://github.com/edlovesjava/DOTplatform/blob/main/hardware/SPEC.md)**

## Quick Reference

| Component | Description |
|-----------|-------------|
| MCU | ATtiny85 (DIP-8) |
| Display | MAX7219 + 8x8 LED matrix |
| Input | 2 buttons (PB3, PB4) with chord detection |
| Power | USB or battery (2xAAA/LiPo) |

## Pin Configuration

| Pin | Function |
|-----|----------|
| PB0 | MAX7219 DIN |
| PB1 | MAX7219 CLK |
| PB2 | MAX7219 CS |
| PB3 | RIGHT button |
| PB4 | LEFT button |
| PB5 | RESET (preserved for USB programming) |

## Controls

| Input | Action |
|-------|--------|
| LEFT button | Move gun left |
| RIGHT button | Move gun right |
| BOTH buttons | Fire bullet |
