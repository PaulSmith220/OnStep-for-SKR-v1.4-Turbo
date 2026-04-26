// HAL setup for LPC176x microcontrollers.
// Initial target: BigTreeTech SKR v1.4 Turbo, LPC1769 at 120MHz.

#define HAL_FAST_PROCESSOR
#define HAL_MAXRATE_LOWER_LIMIT 16
#define HAL_PULSE_WIDTH 500

#include <Arduino.h>
#include <Wire.h>
#include <CDCSerial.h>
#include <usb/cdcuser.h>
#include <usb/usbhw.h>
#include <LPC17xx.h>

#ifndef PI
  #define PI 3.1415926535897932384626433832795
#endif
#ifndef bitRead
  #define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#endif
#ifndef bitSet
  #define bitSet(value, bit) ((value) |= (1UL << (bit)))
#endif
#ifndef bitClear
  #define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#endif
#ifndef bitWrite
  #define bitWrite(value, bit, bitvalue) ((bitvalue) ? bitSet((value), (bit)) : bitClear((value), (bit)))
#endif

#define cli() noInterrupts()
#define sei() interrupts()

// Serial ports -------------------------------------------------------------------------------
#define SerialA UsbSerial
#define SerialB Serial
#define HAL_SERIAL_B_ENABLED
#if SERIAL_C_BAUD_DEFAULT != OFF
  #error "Configuration (Config.h): SerialC is not yet supported on the LPC176x HAL."
#endif

// I2C ----------------------------------------------------------------------------------------
#define HAL_Wire Wire
#define HAL_WIRE_CLOCK 100000

// Non-volatile storage -----------------------------------------------------------------------
// Temporary compile/bring-up backend. Replace with I2C EEPROM/FRAM or flash-backed storage
// before relying on parking, alignment, or runtime configuration persistence.
#include "NV_RAM.h"

// Timing -------------------------------------------------------------------------------------
#define F_COMP 4000000UL
#define ISR(f) void f(void)

void TIMER1_COMPA_vect(void);  // Sidereal timer
void TIMER3_COMPA_vect(void);  // Axis1 RA/Azm timer
void TIMER4_COMPA_vect(void);  // Axis2 DEC/Alt timer

static inline uint32_t lpcPclkFromSelect(uint32_t bits) {
  switch (bits & 0x03) {
    case 0x01: return SystemCoreClock;
    case 0x02: return SystemCoreClock / 2;
    case 0x03: return SystemCoreClock / 8;
    default:   return SystemCoreClock / 4;
  }
}

static inline uint32_t lpcTimerPclk(LPC_TIM_TypeDef *timer) {
  if (timer == LPC_TIM0) return lpcPclkFromSelect((LPC_SC->PCLKSEL0 >> 2) & 0x03);
  if (timer == LPC_TIM1) return lpcPclkFromSelect((LPC_SC->PCLKSEL0 >> 4) & 0x03);
  if (timer == LPC_TIM2) return lpcPclkFromSelect((LPC_SC->PCLKSEL1 >> 12) & 0x03);
  if (timer == LPC_TIM3) return lpcPclkFromSelect((LPC_SC->PCLKSEL1 >> 14) & 0x03);
  return SystemCoreClock / 4;
}

static inline uint32_t lpcPrescaleForRate(LPC_TIM_TypeDef *timer, uint32_t tickHz) {
  uint32_t pclk = lpcTimerPclk(timer);
  uint32_t prescale = (pclk + (tickHz / 2)) / tickHz;
  if (prescale < 1) prescale = 1;
  return prescale - 1;
}

static inline void lpcTimerStart(LPC_TIM_TypeDef *timer, uint32_t tickHz, uint32_t matchTicks) {
  timer->TCR = 0x02;              // Reset
  timer->PR = lpcPrescaleForRate(timer, tickHz);
  timer->MR0 = matchTicks;
  timer->MCR = 0x03;              // Interrupt and reset on MR0
  timer->IR = 0xFF;               // Clear pending flags
  timer->TCR = 0x01;              // Enable
}

static inline uint32_t lpcSafeTicks(uint32_t ticks) {
  return ticks < 2 ? 2 : ticks;
}

static inline void lpcSetAxisTimer(LPC_TIM_TypeDef *timer, uint32_t ticks) {
  timer->MR0 = lpcSafeTicks(ticks);
}

// Nanoseconds delay function -----------------------------------------------------------------
unsigned int _nanosPerPass = 1;
void delayNanoseconds(unsigned int n) {
  unsigned int np = (n / _nanosPerPass);
  for (unsigned int i = 0; i < np; i++) {
    __asm__ volatile ("nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t");
  }
}

void HAL_Initialize(void) {
  USB_Init();
  CDC_Init();
  USB_Connect(TRUE);

  uint32_t startTime, npp;
  startTime = micros();
  delayNanoseconds(65535);
  npp = micros();
  npp = ((int32_t)(npp - startTime) * 1000) / 65535;
  if (npp < 1) npp = 1;
  if (npp > 2000) npp = 2000;
  _nanosPerPass = npp;
}

float HAL_MCU_Temperature(void) {
  return -999;
}

void HAL_Init_Timer_Sidereal() {
  LPC_SC->PCONP |= (1 << 1);      // Timer0 power
  NVIC_SetPriority(TIMER0_IRQn, 2);
  NVIC_EnableIRQ(TIMER0_IRQn);
  lpcTimerStart(LPC_TIM0, 6000000UL, 60000); // Placeholder until Timer1SetInterval() is called
}

void HAL_Init_Timers_Motor() {
  LPC_SC->PCONP |= (1 << 2);      // Timer1 power
  LPC_SC->PCONP |= (1 << 22);     // Timer2 power

  NVIC_SetPriority(TIMER1_IRQn, 0);
  NVIC_SetPriority(TIMER2_IRQn, 0);
  NVIC_EnableIRQ(TIMER1_IRQn);
  NVIC_EnableIRQ(TIMER2_IRQn);

  lpcTimerStart(LPC_TIM1, F_COMP, 65535);
  lpcTimerStart(LPC_TIM2, F_COMP, 65535);
}

void Timer1SetInterval(long iv, double rateRatio) {
  double us = ((double)iv / 16.0) / rateRatio;
  uint32_t ticks = (uint32_t)round(us * 6.0); // Sidereal timer runs at 6MHz.
  LPC_TIM0->MR0 = lpcSafeTicks(ticks);
}

void PresetTimerInterval(long iv, bool TPS, volatile uint32_t *nextRate, volatile uint16_t *nextRep) {
  if (iv > 2144000000) iv = 2144000000;
  if (iv < 16) iv = 16;
  if (!TPS) iv /= 2L;

  uint32_t ticks = (uint32_t)(iv / 4L); // 4MHz motor timers, input is 1/16 us.
  if (ticks < 2) ticks = 2;
  cli();
  *nextRate = ticks;
  *nextRep = 1;
  sei();
}

#define QuickSetIntervalAxis1(r) lpcSetAxisTimer(LPC_TIM1, r)
#define QuickSetIntervalAxis2(r) lpcSetAxisTimer(LPC_TIM2, r)

extern "C" void TIMER0_IRQHandler(void) {
  if (LPC_TIM0->IR & 0x01) {
    LPC_TIM0->IR = 0x01;
    TIMER1_COMPA_vect();
  }
}

extern "C" void TIMER1_IRQHandler(void) {
  if (LPC_TIM1->IR & 0x01) {
    LPC_TIM1->IR = 0x01;
    TIMER3_COMPA_vect();
  }
}

extern "C" void TIMER2_IRQHandler(void) {
  if (LPC_TIM2->IR & 0x01) {
    LPC_TIM2->IR = 0x01;
    TIMER4_COMPA_vect();
  }
}

// Fast port writing help ---------------------------------------------------------------------
#define CLR(x,y) (x &= (~(1 << y)))
#define SET(x,y) (x |= (1 << y))
#define TGL(x,y) (x ^= (1 << y))

#define a1STEP_H digitalWrite(Axis1_STEP, HIGH)
#define a1STEP_L digitalWrite(Axis1_STEP, LOW)
#define a1DIR_H  digitalWrite(Axis1_DIR, HIGH)
#define a1DIR_L  digitalWrite(Axis1_DIR, LOW)

#define a2STEP_H digitalWrite(Axis2_STEP, HIGH)
#define a2STEP_L digitalWrite(Axis2_STEP, LOW)
#define a2DIR_H  digitalWrite(Axis2_DIR, HIGH)
#define a2DIR_L  digitalWrite(Axis2_DIR, LOW)

#define delaySPI delayNanoseconds(500)

#define a1CS_H  digitalWrite(Axis1_M2, HIGH)
#define a1CS_L  digitalWrite(Axis1_M2, LOW)
#define a1CLK_H digitalWrite(Axis1_M1, HIGH)
#define a1CLK_L digitalWrite(Axis1_M1, LOW)
#define a1SDO_H digitalWrite(Axis1_M0, HIGH)
#define a1SDO_L digitalWrite(Axis1_M0, LOW)
#define a1M0(P) digitalWrite(Axis1_M0, (P))
#define a1M1(P) digitalWrite(Axis1_M1, (P))
#define a1M2(P) digitalWrite(Axis1_M2, (P))

#define a2CS_H  digitalWrite(Axis2_M2, HIGH)
#define a2CS_L  digitalWrite(Axis2_M2, LOW)
#define a2CLK_H digitalWrite(Axis2_M1, HIGH)
#define a2CLK_L digitalWrite(Axis2_M1, LOW)
#define a2SDO_H digitalWrite(Axis2_M0, HIGH)
#define a2SDO_L digitalWrite(Axis2_M0, LOW)
#define a2M0(P) digitalWrite(Axis2_M0, (P))
#define a2M1(P) digitalWrite(Axis2_M1, (P))
#define a2M2(P) digitalWrite(Axis2_M2, (P))

#define a3CS_H  digitalWrite(Axis3_M2, HIGH)
#define a3CS_L  digitalWrite(Axis3_M2, LOW)
#define a4CS_H  digitalWrite(Axis4_M2, HIGH)
#define a4CS_L  digitalWrite(Axis4_M2, LOW)
#define a5CS_H  digitalWrite(Axis5_M2, HIGH)
#define a5CS_L  digitalWrite(Axis5_M2, LOW)
