#include <Arduino.h>
#include <CDCSerial.h>
#include <usb/cdcuser.h>
#include <usb/usbhw.h>

// BigTreeTech SKR v1.4 / v1.4 Turbo X driver socket from Marlin pin definitions.
static constexpr pin_t X_ENABLE_PIN = P2_01;
static constexpr pin_t X_STEP_PIN   = P2_02;
static constexpr pin_t X_DIR_PIN    = P2_06;

static bool enabled = true;
static bool direction = false;
static uint32_t lastStepUs = 0;
static uint32_t lastFlipMs = 0;

static void usbStart() {
  USB_Init();
  CDC_Init();
  USB_Connect(TRUE);
}

static void setDriverEnabled(bool on) {
  enabled = on;
  digitalWrite(X_ENABLE_PIN, on ? LOW : HIGH); // TMC/A4988-style modules are normally enable-low.
}

void setup() {
  usbStart();

  pinMode(X_ENABLE_PIN, OUTPUT);
  pinMode(X_STEP_PIN, OUTPUT);
  pinMode(X_DIR_PIN, OUTPUT);

  digitalWrite(X_STEP_PIN, LOW);
  digitalWrite(X_DIR_PIN, LOW);
  setDriverEnabled(true);

  UsbSerial.begin(9600);
}

void loop() {
  while (UsbSerial.available()) {
    const int c = UsbSerial.read();
    if (c == 'e' || c == 'E') setDriverEnabled(true);
    if (c == 'd' || c == 'D') setDriverEnabled(false);
    if (c == 'r' || c == 'R') {
      direction = !direction;
      digitalWrite(X_DIR_PIN, direction ? HIGH : LOW);
    }
    if (c == '?') {
      UsbSerial.print("SKR X motor test: E enable, D disable, R reverse\r\n");
    }
  }

  if (!enabled) return;

  const uint32_t nowUs = micros();
  if ((uint32_t)(nowUs - lastStepUs) >= 1000) {
    lastStepUs = nowUs;
    digitalWrite(X_STEP_PIN, HIGH);
    delayMicroseconds(5);
    digitalWrite(X_STEP_PIN, LOW);
  }

  const uint32_t nowMs = millis();
  if ((uint32_t)(nowMs - lastFlipMs) >= 5000) {
    lastFlipMs = nowMs;
    direction = !direction;
    digitalWrite(X_DIR_PIN, direction ? HIGH : LOW);
  }
}
