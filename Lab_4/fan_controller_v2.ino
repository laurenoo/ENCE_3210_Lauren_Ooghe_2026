// Fan Controller

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED Config
#define SCREEN_WIDTH   128
#define SCREEN_HEIGHT   32
#define OLED_RESET      -1
#define OLED_I2C_ADDR 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pins
#define TEMP_PIN     A0   // TMP36 temperature sensor
#define PWM_PIN       9   // D3 — PWM output to LED (fan simulation)
#define BUTTON1_PIN   2   // D5 — Fan ON/OFF toggle
#define BUTTON2_PIN   3   // D6 — Sensitivity cycle

// ADC & voltages
#define ADC_REF_VOLTAGE   5.0f
#define ADC_RESOLUTION  1023.0f
#define NUM_SAMPLES       100     // 100 samples
#define SAMPLE_INTERVAL    50     // ms between samples (100 × 50ms = 5 sec)

// temp range
#define TEMP_MIN   20.0f   // °C — fan off below this
#define TEMP_MAX   50.0f   // °C — fan at full speed above this

// Button debounce consant
#define DEBOUNCE_MS 200

// Levels of sensitivity
typedef enum {
  SENSITIVITY_LOW    = 0,   // max PWM = 85  (~33%)
  SENSITIVITY_MEDIUM = 1,   // max PWM = 170 (~67%)
  SENSITIVITY_HIGH   = 2    // max PWM = 255 (100%)
} SensitivityLevel;

const int  PWM_MAX[]    = { 85, 170, 255 };
const char* SENS_LABEL[] = { "LOW", "MED", "HI " };

// States
bool             fanEnabled  = false;
SensitivityLevel sensitivity = SENSITIVITY_HIGH;
float            avgTemp     = 0.0f;
int              currentPWM  = 0;

// flags for button ISRs
volatile bool btn1Pressed = false;
volatile bool btn2Pressed = false;

// Pin interrupt
void setupPinChangeInterrupts() {
  // Enable pin change interrupt for Port D (PCIE2)
  PCICR  |= _BV(PCIE2);

  // Enable interrupts specifically on D5 (PCINT21) and D6 (PCINT22)
  PCMSK2 |= _BV(PCINT21) | _BV(PCINT22);
}

// ISR for Port D pin change (fires on any change on D5 or D6)
ISR(PCINT2_vect) {
  static unsigned long lastBtn1 = 0;
  static unsigned long lastBtn2 = 0;
  unsigned long now = millis();

  // Check D2 (Button 1) — LOW means pressed (INPUT_PULLUP)
  if (digitalRead(BUTTON1_PIN) == LOW && (now - lastBtn1) > DEBOUNCE_MS) {
    lastBtn1 = now;
    btn1Pressed = true;
  }

  // Check D6 (Button 2) — LOW means pressed (INPUT_PULLUP)
  if (digitalRead(BUTTON2_PIN) == LOW && (now - lastBtn2) > DEBOUNCE_MS) {
    lastBtn2 = now;
    btn2Pressed = true;
  }
}

//  Processes button flags set by the ISR
void handleButtons() {
  if (btn1Pressed) {
    btn1Pressed = false;
    fanEnabled = !fanEnabled;

    if (!fanEnabled) {
      currentPWM = 0;
      analogWrite(PWM_PIN, 0);
    } else {
      // Immediately apply last known PWM when turning on
      analogWrite(PWM_PIN, currentPWM);
    }

    updateOLED();   // refresh display on button press
  }

  if (btn2Pressed) {
    btn2Pressed = false;
    sensitivity = (SensitivityLevel)((sensitivity + 1) % 3);

    // Recalculate PWM immediately with new sensitivity
    if (fanEnabled) {
      currentPWM = tempToPWM(avgTemp);
      analogWrite(PWM_PIN, currentPWM);
    }

    updateOLED();   // ← refresh display instantly on button press
  }
}

// ============================================================
//  readTemperature()
//  Spec (b): 100 samples over 5 seconds, returns avg in °C
// ============================================================
float readTemperature() {
  long sum = 0;
  for (int i = 0; i < NUM_SAMPLES; i++) {
    sum += analogRead(TEMP_PIN);
    handleButtons();   // ← checks flags every 50ms during sampling

    delay(SAMPLE_INTERVAL);
  }
  float avg     = sum / (float)NUM_SAMPLES;
  float voltage = (avg / ADC_RESOLUTION) * ADC_REF_VOLTAGE;
  return (voltage - 0.5f) / 0.01f;   // TMP36: °C = (V - 0.5) / 0.01
}

// set PWM based on temp value
int tempToPWM(float temp) {
  int maxPWM = PWM_MAX[sensitivity];
  if (temp <= TEMP_MIN) return 0;
  if (temp >= TEMP_MAX) return maxPWM;
  return (int)((temp - TEMP_MIN) / (TEMP_MAX - TEMP_MIN) * maxPWM);
}

// OLED update function
void updateOLED() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  // Line 1 — temperature and PWM
  display.setCursor(0, 0);
  display.print(F("TEMP:"));
  display.print(avgTemp, 1);
  display.print(F("C  PWM:"));
  display.print(currentPWM);

  // Line 2 — fan status and sensitivity
  display.setCursor(0, 11);
  display.print(F("FAN:"));
  display.print(fanEnabled ? F("ON  ") : F("OFF "));
  display.print(F("SENS:"));
  display.print(SENS_LABEL[sensitivity]);

  // Line 3 — speed bar
  display.setCursor(0, 22);
  display.print(F("SPD:"));
  int barWidth = map(currentPWM, 0, 255, 0, 100);
  display.drawRect(28, 23, 100, 7, SSD1306_WHITE);
  if (barWidth > 0) {
    display.fillRect(28, 23, barWidth, 7, SSD1306_WHITE);
  }

  display.display();
}

void setup() {
  // Button pins with internal pullup, reads LOW when pressed
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(PWM_PIN, OUTPUT);
  analogWrite(PWM_PIN, 0);

  // Set up pin change interrupts for D5 and D6
  setupPinChangeInterrupts();

  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR)) {
    // OLED failed — blink LED to signal error
    pinMode(LED_BUILTIN, OUTPUT);
    while (true) {
      digitalWrite(LED_BUILTIN, HIGH); delay(200);
      digitalWrite(LED_BUILTIN, LOW);  delay(200);
    }
  }

  // Splash screen
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(20, 0);
  display.print(F("FAN CONTROLLER"));
  display.setCursor(10, 11);
  display.print(F("BTN1:On/Off BTN2:Sens"));
  display.setCursor(25, 22);
  display.print(F("Initializing..."));
  display.display();
  delay(2000);
}

// Main loop function
void loop() {

  // Handle any button presses
  handleButtons();

  //  temperature (100 samples / 5 sec)
  avgTemp = readTemperature();

  // Calculate PWM from temperature
  if (fanEnabled) {
    currentPWM = tempToPWM(avgTemp);
  } else {
    currentPWM = 0;
  }

  // Write PWM to LED (spec g — simulates fan)
  analogWrite(PWM_PIN, currentPWM);

  // update OLED
  updateOLED();
}
