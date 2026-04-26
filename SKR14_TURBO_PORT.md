# OnStep SKR V1.4 Turbo Port

This is an experimental classic OnStep 4.24 port for the BigTreeTech SKR V1.4 Turbo.

## Current Hardware Target

- Board: BigTreeTech SKR V1.4 Turbo
- MCU: NXP LPC1769, Cortex-M3, 120MHz
- Build target: `platformio run -e skr14turbo`
- Pinmap: `BTT_SKR14_TURBO`

## Initial Axis Mapping

- Axis 1 RA/Azm: SKR X driver socket
- Axis 2 Dec/Alt: SKR Y driver socket
- Axis 3 rotator: SKR Z driver socket
- Axis 4 focuser 1: SKR E0 driver socket
- Axis 5 focuser 2: SKR E1 driver socket

The pin assignments are derived from Marlin's SKR V1.4 pin definitions. SKR V1.4 and SKR V1.4 Turbo share the same board pinout; the Turbo uses the LPC1769.

## Status

- Added OnStep pinmap ID `BTT_SKR14_TURBO`.
- Added experimental LPC176x HAL with direct LPC timer setup for sidereal, Axis1, and Axis2 interrupts.
- Added PlatformIO environment for the LPC1769 Arduino core.
- Configured `Config.h` for a minimal two-axis generic step/dir build.

## Known Risks

- This has not been bench tested on the board.
- Timer behavior must be verified with a scope or logic analyzer before motors are connected.
- Non-volatile settings should use an external I2C EEPROM or FRAM. LPC1769 has no onboard EEPROM.
- TMC UART/SPI current control is not enabled yet. Start with standalone driver mode.
- ST4 and accessory pins need electrical verification before connecting external equipment.

## First Bench Test

1. Build firmware.
2. Flash to SD card using the normal SKR `firmware.bin` workflow.
3. Boot with motors disconnected.
4. Confirm serial command response over USB.
5. Scope X STEP/DIR and Y STEP/DIR during manual moves.
6. Only then connect drivers/motors with conservative current limits.
