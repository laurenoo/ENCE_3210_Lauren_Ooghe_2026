/*
  100 Hz Ramp Test for 4-bit DAC
  Pins: 3,4,5,6  (LSB -> MSB)
*/

const uint8_t dacPins[4] = {3, 4, 5, 6};

inline void writeDAC4(uint8_t code) {
  code &= 0x0F;
  digitalWrite(dacPins[0], (code >> 0) & 1);
  digitalWrite(dacPins[1], (code >> 1) & 1);
  digitalWrite(dacPins[2], (code >> 2) & 1);
  digitalWrite(dacPins[3], (code >> 3) & 1);
}

void setup() {
  for (int i = 0; i < 4; i++) {
    pinMode(dacPins[i], OUTPUT);
    digitalWrite(dacPins[i], LOW);
  }
}

void loop() {
  const unsigned int stepDelay_us = 313;   //  µs per code

  for (uint8_t code = 0; code < 16; code++) {
    writeDAC4(code);
    delayMicroseconds(stepDelay_us);
  }
}

