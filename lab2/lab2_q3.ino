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

volatile unsigned char gFlag_1 = 0;   // create interrupt flags, volatile because they are used in different places at same tiem
volatile unsigned char gFlag_2 = 0;


// LED pattern 
const byte ledPatt[5][3] = {
  {0,0,0},  // all LEDs off
  {1,0,0},  // addition red LED
  {0,1,0},  // subtraction green LED
  {0,0,1},  // multiplication blue LED
  {1,1,1}   // gOperation done, all LEDs on
};


// fucntion to set LEDs on/off according to sequence
void setLED(int mode) {
  digitalWrite(R_LED, ledPatt[mode][0]);
  digitalWrite(G_LED, ledPatt[mode][1]);
  digitalWrite(B_LED, ledPatt[mode][2]);
}

void setup() {
  Serial.begin(9600);    // begin serial monitoring
  pinMode(R_LED, OUTPUT);    // set LEDs as OUTPUTs
  pinMode(G_LED, OUTPUT);
  pinMode(B_LED, OUTPUT);
  
  pinMode(BTN_SEL, INPUT_PULLUP);    // set buttons as inputs
  pinMode(BTN_EXE, INPUT_PULLUP);

  setLED(0);  // set LEDs off initially
  // add interrupts so when button is pressed, it will run and set the flag to 1 to run code
  attachInterrupt(digitalPinToInterrupt(BTN_SEL), isrButton1, FALLING);
  attachInterrupt(digitalPinToInterrupt(BTN_EXE), isrButton2, FALLING);    
}
// debounce function 
bool edgeDebounced(int pin, bool &lastState, unsigned long &lastTime) {
  bool current = digitalRead(pin);  // read if button is pressed

  if (lastState == HIGH && current == LOW) {  // delay while button press to ensure it only counts one press
    if (millis() - lastTime > debounce) {
      lastTime = millis();
      lastState = current;
      return true;
    }
  }

  lastState = current;  
  return false;
}
// function to compute mathmatical operation 
long arithmetic(int x, int y, int gOperation) {
  switch (gOperation) {
    case 1: return (long)x + (long)y; // addition
    case 2: return (long)x - (long)y; // subtraction
    case 3: return (long)x * (long)y; // multiplication
    default: return 0;
  }
}

void loop() {
 if (gFlag_1){  // if the first button is pressed, the ISR will set flag to 1 and then the following code runs
  if(edgeDebounced(BTN_SEL, gLastSel, gLastSTime)) {    // if the debounce returns true
    gOperation++;    // increase operation to cycle through add, subtract, multiply
    if (gOperation > 3) gOperation = 0;    // reset if all 3 operations are run through already
    setLED(gOperation);  // set the LEDs to the correct sequence for a given operation
    
    // prints what operation is selected
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
  // second flag operates the same as the first, just with the other button
  if(gFlag_2){

    if(edgeDebounced(BTN_EXE, gLastOp, gLastETime)) {  // if debounce returns true, run the code

    if(gOperation == 0) {    // if user pressed execute button without selecting an operation, tell try again
      Serial.print("No input received. Try again. ");
      return;
    }
    setLED(0);  // set LEDs to off
      
    // save results from a given operation into the result array
    for (int i = 0; i < N; i++) {
      result[i] = arithmetic(num1[i], num2[i], gOperation);
    }
    setLED(4); // all LEDs on, when program is done
    // print the results array
    Serial.println("Results: ");
    for (int i = 0; i < N; i++) {
      Serial.println(result[i]);
    }
    }
  }
}
// ISR functions so once button press is detected, the ISR will interrupt and set flags to 1 to run code
void isrButton1(){
  gFlag_1 = 1;
}
void isrButton2() {
  gFlag_2 = 1;
}
