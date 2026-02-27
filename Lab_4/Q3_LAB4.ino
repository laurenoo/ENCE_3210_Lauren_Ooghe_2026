// Q3 - Solar charger using DAC shield for Arduino
// If solar power is above 2.5 V, solar is used to power microcontroller
// If solar power is below 2.5 V, battery power used
// If battery is less than solar, battery will charge on solar power
// Battery can reach a fully charged state, where it then will stop charging

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ── OLED Configuration ──────────────────────────────────────
#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT    32
#define OLED_RESET       -1
#define OLED_I2C_ADDR  0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pin Definitions
#define SOLAR_PIN      A0
#define BATTERY_PIN    A1
#define SWITCH_PIN      8

// ADC Constants
#define ADC_REF_VOLTAGE   5.0f
#define ADC_RESOLUTION  1023.0f

// Voltage thresholds
#define SOLAR_THRESHOLD       2.5f   // below this, use battery for MCU
#define BATTERY_FULL_VOLTAGE  4.2f   // stop charging at this voltage
#define BATTERY_LOW_VOLTAGE   3.0f   // start charging again at this voltage

// Simulation battery charging
#define SIMULATION_MODE     true     // set false to use real ADC reads
#define CHARGE_INCREMENT    0.05f    // volts added per loop when charging
#define DISCHARGE_INCREMENT 0.01f    // volts lost per loop when not charging
#define SIM_SOLAR_VOLTAGE   3.8f     // fixed simulated solar voltage

// enums
typedef enum {
  SOURCE_SOLAR,
  SOURCE_BATTERY
} PowerSource;

typedef enum {
  CHARGE_OFF,
  CHARGE_ON
} ChargeState;

// State
float        vSolar        = 0.0f;  // begin at 0
float        vBattery      = 0.0f;
PowerSource  currentSource = SOURCE_BATTERY;  
ChargeState  chargeState   = CHARGE_OFF;

// Simulation variables
float simBatteryVoltage = BATTERY_LOW_VOLTAGE;
float simSolarVoltage   = SIM_SOLAR_VOLTAGE;

// Calibration scales
float solarScale   = 1.0f;
float batteryScale = 1.0f;

//  Function reads ADC pin with 5-sample averaging and applies scale to get output voltage

float readVoltage(int pin, float scale) {
  long sum = 0;
  for (int i = 0; i < 5; i++) {
    sum += analogRead(pin);
    delay(2);
  }
  float avg     = sum / 5.0f;
  float adcVolt = (avg / ADC_RESOLUTION) * ADC_REF_VOLTAGE;
  return adcVolt * scale;
}

//  Before calling this, apply a known voltage to A0 and A1
//  and set SOLAR_ACTUAL_VOLTAGE and BATTERY_ACTUAL_VOLTAGE.
/*
void calibrate() {
  // ── Set these to the actual voltage you are feeding in ──
  float solarActual   = 5.0f;   // measured with multimeter at solar input
  float batteryActual = 5.0f;   // measured with multimeter at battery input

  float rawSolar   = (analogRead(SOLAR_PIN)   / ADC_RESOLUTION) * ADC_REF_VOLTAGE;
  float rawBattery = (analogRead(BATTERY_PIN) / ADC_RESOLUTION) * ADC_REF_VOLTAGE;

  // Avoid division by zero
  solarScale   = (rawSolar   > 0.1f) ? solarActual   / rawSolar   : 1.0f;
  batteryScale = (rawBattery > 0.1f) ? batteryActual / rawBattery : 1.0f;

  Serial.print(F("Calibration — Solar scale: "));
  Serial.print(solarScale, 3);
  Serial.print(F("  Battery scale: "));
  Serial.println(batteryScale, 3);
}
*/

// Update the simulation
void updateSimulation() {
  // read potentiometer on A0
  vSolar = readVoltage(SOLAR_PIN, solarScale);

  // Battery - simulated, increment/decrement based on charge state
  if (chargeState == CHARGE_ON) { // if charge state is on, battery will increment until reaching fully charged voltage 4.2V
    simBatteryVoltage += CHARGE_INCREMENT;
    if (simBatteryVoltage > BATTERY_FULL_VOLTAGE) {
      simBatteryVoltage = BATTERY_FULL_VOLTAGE;
    }
  } else {
    simBatteryVoltage -= DISCHARGE_INCREMENT;   // simulates battery discharging
    if (simBatteryVoltage < 2.5f) {
      simBatteryVoltage = 2.5f;
    }
  }
  vBattery = simBatteryVoltage;
}

// OLED simulation update
void updateOLED(float vSol, float vBat,
                PowerSource src, ChargeState chg) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  // voltage readings
  display.setCursor(0, 0);
  display.print(F("SOL:"));
  display.print(vSol, 1);
  display.print(F("V BAT:"));
  display.print(vBat, 1);
  display.print(F("V"));

  // MCU power source
  display.setCursor(0, 11);
  display.print(F("MCU: "));
  if (src == SOURCE_SOLAR) {
    display.print(F("SOLAR PANEL"));
  } else {
    display.print(F("BATTERY"));
  }

  // charging status
  display.setCursor(0, 21);
  display.print(F("CHG: "));
  if (chg == CHARGE_ON) {
    display.print(F("ON  SOL->BAT"));
  } else {
    if (vBat >= BATTERY_FULL_VOLTAGE) {
      display.print(F("OFF  FULL"));
    } else {
      display.print(F("OFF"));
    }
  }

  display.display();
}

// prints to serial port at 9600 baud
void printSerial(float vSol, float vBat, PowerSource src, ChargeState chg) {
  Serial.print(F("V_Solar="));
  Serial.print(vSol, 2);
  Serial.print(F("V  |  V_Battery="));
  Serial.print(vBat, 2);
  Serial.print(F("V  |  MCU="));
  Serial.print(src == SOURCE_SOLAR ? F("SOLAR") : F("BATTERY"));
  Serial.print(F("  |  Charging="));
  Serial.println(chg == CHARGE_ON ? F("ON") : F("OFF"));
}


void setup() {
  Serial.begin(9600);   //begin serial monitoring
  Serial.println(F("Solar Charge Controller — Starting..."));   // prints setup

  pinMode(SWITCH_PIN, OUTPUT);  // GPIO 
  digitalWrite(SWITCH_PIN, LOW);


  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR)) {
    Serial.println(F("ERROR: OLED not found!"));
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
  display.setCursor(0, 0);
  display.println(F("SOLAR CHARGE W/ BATT-LAB 4"));
  display.setCursor(0, 11);

  display.setCursor(0, 21);
  display.println(F("Initializing..."));
  display.display();
  delay(4000);


  Serial.println(F("Ready."));
}

void loop() {

  // Get voltages
  if (SIMULATION_MODE) {
    updateSimulation();   // solar from pot A0 and the battery is simulated
  } else {
    vSolar   = readVoltage(SOLAR_PIN,   solarScale);
    vBattery = readVoltage(BATTERY_PIN, batteryScale);
  }

  // Determine MCU power source
  if (vSolar > SOLAR_THRESHOLD) {
    currentSource = SOURCE_SOLAR;
  } else {
    currentSource = SOURCE_BATTERY;
  }

  // Charging logic 
  if (chargeState == CHARGE_ON) {
    // Currently charging — only stop when battery is FULL
    if (vBattery >= BATTERY_FULL_VOLTAGE) {
      chargeState = CHARGE_OFF;
      digitalWrite(SWITCH_PIN, LOW);
      Serial.println(F(">> Battery full — charging stopped."));
    } else {
      digitalWrite(SWITCH_PIN, HIGH);   // keep charging
    }

  } else {
    // Not charging: only start when battery is LOW and solar has power
    if (vBattery < BATTERY_LOW_VOLTAGE && vSolar > SOLAR_THRESHOLD) {
      chargeState = CHARGE_ON;
      digitalWrite(SWITCH_PIN, HIGH);
      Serial.println(F(">> Battery low — charging started."));  //print
    } else {
      digitalWrite(SWITCH_PIN, LOW);
    }
  }

  // Update OLED display with current values
  updateOLED(vSolar, vBattery, currentSource, chargeState);

  //print to serial port
  printSerial(vSolar, vBattery, currentSource, chargeState);

  delay(500);
}
