#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/* ================= OLED CONFIG ================= */
#define SCREEN_WIDTH    128   // OLED display width, in pixels
#define SCREEN_HEIGHT   32    // OLED display height, in pixels
#define OLED_RESET      -1    // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS  0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/* ================= GPIOs ================= */
#define BUTTON_1 2
#define BUTTON_2 3

// DAC uses:
// PORTD bits 4–7  -> DAC bits 0–3
// PORTB bits 0–5  -> DAC bits 4–9
const int gDAC_PIN[10] = {4, 5, 6, 7, 8, 9, 10, 11, 12, 13};

/* ================= SIGNAL ================= */
#define SINE_SAMPLES  256

uint16_t gSineTable[SINE_SAMPLES];
uint16_t gSampleDelayUs = 1;

/* ================= MENU ================= */
enum MenuItem {
  MENU_SINE,
  MENU_SAW,
  MENU_SQUARE,
  MENU_TRIANGLE
};

#define MENU_COUNT 4

const char* menuText[MENU_COUNT] = {
  "SINE",
  "SAWTOOTH",
  "SQUARE",
  "TRIANGLE"
};

int8_t gMenuIndex = 1;

volatile uint8_t gISR_Flag_1 = 0;
volatile uint8_t gISR_Flag_2 = 0;

void setup() {
  // Init Buttons
  pinMode(BUTTON_1, INPUT);
  pinMode(BUTTON_2, INPUT);

  attachInterrupt(digitalPinToInterrupt(BUTTON_1), isr_button_1, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_2), isr_button_2, FALLING);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.setTextColor(SSD1306_WHITE); // Set OLED Text Color
  display.clearDisplay(); // Clear the buffer
  display.setRotation(0);  // Set screen rotation

  // Init DAC
  // Init GPIO
  for(int i=0; i<10; i++){
    pinMode(gDAC_PIN[i], OUTPUT);
  }

  // Generate sine lookup table (0–1023)
  for (int i = 0; i < SINE_SAMPLES; i++) {
    float angle = 2.0f * PI * i / SINE_SAMPLES;
    gSineTable[i] = (uint16_t)(512 + 511 * sin(angle));
  }

  outputDAC(0);
}

/* ================= LOOP ================= */
void loop() {
  if( (gISR_Flag_1 == 1) ||(gISR_Flag_2 == 1) )
    updateDisplay();
  
  generateWave();
}

/* ================= FUNCTIONS ================= */
void isr_button_1(){
  gISR_Flag_1 = 1;
}

void isr_button_2(){
  gISR_Flag_2 = 1;
}

void updateDisplay() {

  if(gISR_Flag_1) {
    gISR_Flag_1 = 0;
    gMenuIndex = (gMenuIndex - 1 + MENU_COUNT) % MENU_COUNT;
  }

  if(gISR_Flag_2) {
    gISR_Flag_2 = 0;
    gMenuIndex = (gMenuIndex + 1) % MENU_COUNT;
  }

  display.clearDisplay();
  display.setTextSize(1);

  for (int8_t i = -1; i <= 1; i++) {
    int8_t item = (gMenuIndex + i + MENU_COUNT) % MENU_COUNT;
    int y = (i + 1) * 10;

    display.setCursor(0, y);
    display.print(i == 0 ? "> " : "  ");
    display.print(menuText[item]);
  }

  display.display();
}

void generateWave() {

  switch (gMenuIndex) {
    case MENU_SINE:
      // Sine Wave
      for (uint16_t i = 0; i < SINE_SAMPLES; i++) {
        outputDAC(gSineTable[i]);
        delayMicroseconds(gSampleDelayUs);
      }
      break;
    
    case MENU_SAW:
      // Sawtooth Wave
      for(int i=0; i<1024; i++){
        outputDAC(i);
        delayMicroseconds(gSampleDelayUs);
      }
      break;

    case MENU_SQUARE:
      // Square Wave
      outputDAC(0);          // LOW = 0V
      delayMicroseconds(256 * gSampleDelayUs);
      
      outputDAC(0x3FF);
      delayMicroseconds(256 * gSampleDelayUs);
      break;

    case MENU_TRIANGLE:
      // Triangle Wave: 0 → 1023 → 0
      for (uint16_t i = 0; i < 1023; i++) {
        outputDAC(i);
        delayMicroseconds(gSampleDelayUs);
      }
      for (int16_t i = 1023; i > 0; i--) {
        outputDAC(i);
        delayMicroseconds(gSampleDelayUs);
      }
      break;
  }
}

void outputDAC(uint16_t value) {
  value &= 0x03FF; // 10-bit safety mask
  // PORTD: bits 4–7 = DAC bits 0–3
  PORTD = (PORTD & 0x0F) | ((value & 0x000F) << 4);
  // PORTB: bits 0–5 = DAC bits 4–9
  PORTB = (PORTB & 0xC0) | (value >> 4);
}

