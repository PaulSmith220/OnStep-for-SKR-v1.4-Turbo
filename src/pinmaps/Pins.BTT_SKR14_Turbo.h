// -------------------------------------------------------------------------------------------------
// Experimental pin map for OnStep using BigTreeTech SKR v1.4 Turbo LPC1769 pin defs.
//
// Pin assignments are based on Marlin's BTT SKR v1.4 pin map:
// https://github.com/MarlinFirmware/Marlin/blob/bugfix-2.1.x/Marlin/src/pins/lpc1768/pins_BTT_SKR_V1_4.h

#if defined(__LPC1769__) || defined(LPC1769) || defined(TARGET_LPC1769) || defined(ARDUINO_ARCH_LPC176X)

// SKR v1.4 and v1.4 Turbo share the same board pinout; Turbo uses LPC1769.

// Auxiliary / exposed pins
#define Aux0              P1_24     // Neopixel header, usable as status LED output with suitable wiring
#define Aux1              P2_04     // Fan0
#define Aux2              P2_03     // Fan1
#define Aux3              P2_05     // Bed heater MOSFET
#define Aux4              P2_07     // Hotend heater MOSFET
#define Aux5              P1_23     // EXP1
#define Aux6              P1_22     // EXP1
#define Aux7              P1_21     // EXP1
#define Aux8              P1_20     // EXP1

// Default software SPI pins for TMC drivers.
#define SSPI_MOSI         P1_17
#define SSPI_MISO         P0_05
#define SSPI_SCK          P0_04

// Sensors and user feedback.
#define PecPin            P1_26     // E0DET
#define AnalogPecPin      OFF
#define LimitPin          P1_27     // Z-STOP
#define LEDnegPin         Aux0
#define TonePin           P1_21     // EXP1 pin 6
#define PpsPin            P1_25     // E1DET

// Axis1 RA/Azm on X driver socket.
#define Axis1_EN          P2_01
#define Axis1_M0          SSPI_MOSI
#define Axis1_M1          SSPI_SCK
#define Axis1_M2          P1_10     // X CS / UART
#define Axis1_M3          SSPI_MISO
#define Axis1_STEP        P2_02
#define Axis1_DIR         P2_06
#define Axis1_DECAY       Axis1_M2
#define Axis1_FAULT       Axis1_M3
#define Axis1_HOME        P1_29     // X-STOP

// Axis2 Dec/Alt on Y driver socket.
#define Axis2_EN          P2_08
#define Axis2_M0          SSPI_MOSI
#define Axis2_M1          SSPI_SCK
#define Axis2_M2          P1_09     // Y CS / UART
#define Axis2_M3          SSPI_MISO
#define Axis2_STEP        P0_19
#define Axis2_DIR         P0_20
#define Axis2_DECAY       Axis2_M2
#define Axis2_FAULT       Axis2_M3
#define Axis2_HOME        P1_28     // Y-STOP

// Axis3 rotator on Z driver socket.
#define Axis3_EN          P0_21
#define Axis3_M0          SSPI_MOSI
#define Axis3_M1          SSPI_SCK
#define Axis3_M2          P1_08     // Z CS / UART
#define Axis3_M3          SSPI_MISO
#define Axis3_STEP        P0_22
#define Axis3_DIR         P2_11

// Axis4 focuser 1 on E0 driver socket.
#define Axis4_EN          P2_12
#define Axis4_M0          SSPI_MOSI
#define Axis4_M1          SSPI_SCK
#define Axis4_M2          P1_04     // E0 CS / UART
#define Axis4_M3          SSPI_MISO
#define Axis4_STEP        P2_13
#define Axis4_DIR         P0_11

// Axis5 focuser 2 on E1 driver socket.
#define Axis5_EN          P1_16
#define Axis5_M0          SSPI_MOSI
#define Axis5_M1          SSPI_SCK
#define Axis5_M2          P1_01     // E1 CS / UART
#define Axis5_M3          SSPI_MISO
#define Axis5_STEP        P1_15
#define Axis5_DIR         P1_14

// ST4 interface on EXP headers. Verify voltage levels before connecting external guider hardware.
#define ST4RAw            P1_23
#define ST4DEs            P1_22
#define ST4DEn            P1_20
#define ST4RAe            P1_18

#else
#error "Wrong processor for this configuration!"
#endif
