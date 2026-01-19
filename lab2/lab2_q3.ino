// Lab 2 - Q3 - Lauren Ooghe
// LED calculator with interrupt

#define N 10   // size of array

// assign LED pins
int R_LED = 5;
int G_LED = 4;
int B_LED = 11;

// assign button pins
int BTN_SEL = 2;
int BTN_EXE = 3;

// create 2 integer filled arrays and one for the results to be stored in
int num1[N] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
int num2[N] = {2, 3, 4, 1, 5, 8, 7, 9, 4, 5};
long result[N];


int gOperation = 0;   // variable for what arithmetic operation to perform (will toggle between 0-3)

bool gLastSel = HIGH;   // variable for comparison between current button press and last button press for debounce
bool gLastOp = HIGH;    // comparison value between curent execution button press and last press for debounce function

unsigned long gLastSTime = 0;   // time button select compare value
unsigned long gLastETime = 0;   // time button execution compare value

const unsigned long debounce = 150;   // 150 ms debounce value

volatile unsigned char gFlag_1 = 0;   //
volatile unsigned char gFlag_2 = 0;


const byte ledPatt[5][3] = {
  {0,0,0},  // all LEDs off
  {1,0,0},  // addition red LED
  {0,1,0},  // subtraction green LED
  {0,0,1},  // multiplication blue LED
  {1,1,1}   // gOperation done, all LEDs on
};



void setLED(int mode) {
  digitalWrite(R_LED, ledPatt[mode][0]);
  digitalWrite(G_LED, ledPatt[mode][1]);
  digitalWrite(B_LED, ledPatt[mode][2]);
}

void setup() {
  Serial.begin(9600);
  pinMode(R_LED, OUTPUT);
  pinMode(G_LED, OUTPUT);
  pinMode(B_LED, OUTPUT);
  
  pinMode(BTN_SEL, INPUT_PULLUP);
  pinMode(BTN_EXE, INPUT_PULLUP);

  setLED(0);

  attachInterrupt(digitalPinToInterrupt(BTN_SEL), isrButton1, FALLING);
  attachInterrupt(digitalPinToInterrupt(BTN_EXE), isrButton2, FALLING);    // add interrupt so when button is pressed
}
bool edgeDebounced(int pin, bool &lastState, unsigned long &lastTime) {
  bool current = digitalRead(pin);

  if (lastState == HIGH && current == LOW) {
    if (millis() - lastTime > debounce) {
      lastTime = millis();
      lastState = current;
      return true;
    }
  }

  lastState = current;
  return false;
}
long arithmetic(int x, int y, int gOperation) {
  switch (gOperation) {
    case 1: return (long)x + (long)y; // addition
    case 2: return (long)x - (long)y; // subtraction
    case 3: return (long)x * (long)y; // multiplication
    default: return 0;
  }
}
void loop() {
 if (gFlag_1){
  if(edgeDebounced(BTN_SEL, gLastSel, gLastSTime)) {
    gOperation++;
    if (gOperation > 3) gOperation = 0;
    setLED(gOperation);
    if (gOperation==1){
      Serial.println("Addition");
    }
    if (gOperation > 1 && gOperation < 3){
      Serial.println("Subtraction");
    }
    if (gOperation > 2 && gOperation == 3){
      Serial.println("Multiplication");
    }
  }
 }
  if(gFlag_2){

    if(edgeDebounced(BTN_EXE, gLastOp, gLastETime)) {

    if(gOperation == 0) {
      Serial.print("No input received. Try again. ");
      return;
    }
    setLED(0);

    for (int i = 0; i < N; i++) {
      result[i] = arithmetic(num1[i], num2[i], gOperation);
    }
    setLED(4); // all LEDs on, when program is done

    Serial.println("Results: ");
    for (int i = 0; i < N; i++) {
      Serial.println(result[i]);
    }
    }
  }
}
void isrButton1(){
  gFlag_1 = 1;
}
void isrButton2() {
  gFlag_2 = 1;
}
