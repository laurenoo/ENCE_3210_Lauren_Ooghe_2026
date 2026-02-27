// Q2 - Solar charger using DAC shield for Arduino
// If solar power is above 2.5 V, solar is used to power microcontroller
// If solar power is below 2.5 V, battery power used
// If battery is less than solar, battery will charge on solar power

// include all needed libraries
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED Setup using SSD1306 OLED screen 128x32
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT  32
#define OLED_RESET     -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Define pins
#define SOLAR_PIN   A0   // ADC pin for solar panel voltage
#define BATTERY_PIN A1   // ADC pin for battery voltage
#define SWITCH_PIN   8   // digital pin to control switching element

// define global variable
#define SOLAR_THRESHOLD  2.5f   // Volts: threshold for power source selection
#define ADC_REF_VOLTAGE  5.0f   // Reference voltage of ADC powered by Arduino 5V
#define ADC_RESOLUTION  1023.0f // 10-bit ADC

// Voltage divider - two 100k potentiometers used to control A0 and A1 voltages
#define SOLAR_SCALE   1.0f   // divider scales 5V panel
#define BATTERY_SCALE 1.0f   // divider scales 5V battery

// State Tracking
typedef enum {    // enumerate what state the controller is currently in
  SOURCE_SOLAR,
  SOURCE_BATTERY
} PowerSource;

typedef enum {  // enumerate what state the controller is currently in
  CHARGE_OFF,
  CHARGE_ON
} ChargeState;

// Function that reads the analog pin and converts it to a voltage read out using 1023 bit division multiplied by 5V reference from Arduino
float readVoltage(int pin, float scale) {
  int raw = analogRead(pin);  // reads analog value of potentiometer
  float voltage = (raw / ADC_RESOLUTION) * ADC_REF_VOLTAGE * scale;
  return voltage;
}

// OLED screen update
void updateOLED(float vSol, float vBat, PowerSource src, ChargeState chg) {

  display.clearDisplay();   // clear display before showing anything

  // Voltage readings
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print(F("SOL:"));
  display.print(vSol, 1);
  display.print(F("V BAT:"));
  display.print(vBat, 1);
  display.print(F("V"));

  // MCU power source display
  display.setCursor(0, 10);
  display.print(F("MCU: "));
  if (src == SOURCE_SOLAR) {
    display.print(F("SOLAR PANEL"));
  } else {
    display.print(F("BATTERY"));
  }

  // Charging status display
  display.setCursor(0, 20);
  display.print(F("CHG: "));
  if (chg == CHARGE_ON) {
    display.print(F("ON  SOL->BAT"));
  } else {
    display.print(F("OFF"));
  }

  display.display();
}

// ────────────────────────────────────────────────────────
void setup() {
  Serial.begin(9600);  // baud rate
  pinMode(SWITCH_PIN, OUTPUT);  //define switch pin as output
  digitalWrite(SWITCH_PIN, LOW); // switch open (no charging)

  // checks if OLED is connected
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 OLED not found!")); 
    while (true); // Stop
  }
  // Initialize the screen
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 25);
  display.println(F("Initializing..."));
  display.display();
  delay(1500);
}

// ────────────────────────────────────────────────────────
void loop() {
  // Read voltages invoking readVoltage function that takes the analog input
  float vSolar   = readVoltage(SOLAR_PIN,   SOLAR_SCALE);
  float vBattery = readVoltage(BATTERY_PIN, BATTERY_SCALE);

  // Determine MCU power source
  PowerSource currentSource;  // from enums
  if (vSolar > SOLAR_THRESHOLD) {   // if the solar voltage is greater than the 2.5V
    currentSource = SOURCE_SOLAR;   // source to be solar
    
  } else {    // if not, use battery source
    currentSource = SOURCE_BATTERY;
  }

  // Determine charging state
  ChargeState chargeState;  // using enums
  if (vBattery < vSolar) {    // if batt voltage is less than solar, we want to charge the battery
    chargeState = CHARGE_ON;   // turn on batt charging
    digitalWrite(SWITCH_PIN, HIGH); // closes the switch and connect battery to solar
  } else {  // if batt is higher than solar, charging off
    chargeState = CHARGE_OFF; 
    digitalWrite(SWITCH_PIN, LOW);  // Open the switch and no charging
  }

  // Update OLED display with new values
  updateOLED(vSolar, vBattery, currentSource, chargeState);

  delay(500); // Update screen every 500ms
}
