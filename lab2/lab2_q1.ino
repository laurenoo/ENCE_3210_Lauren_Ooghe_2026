#define gButton1 2    // assign button pins
#define gButton2 3

// assign all global variables 
// assign LED pins
int gR_LED = 5;
int gG_LED = 4;
int gB_LED = 11;

// assign counters
int gCounter1 = 0;
int gCounter2 = 0;

// create interrupt flags, set to zero
volatile char gFlag_1 = 0;
volatile char gFlag_2 = 0;

// debouncing time set to 100 ms for button debounce
const int gDebounce = 100;   // 100ms

// time used to compare in debounce application
unsigned long gLastTime1 = 0;
unsigned long gLastTime2 = 0;


void setup() {

  Serial.begin(9600);     // initialize serial monitoring
  // assign inputs and outputs
  pinMode(gR_LED, OUTPUT);    
  pinMode(gG_LED, OUTPUT);
  pinMode(gB_LED, OUTPUT);
  pinMode(gButton1, INPUT);
  pinMode(gButton2, INPUT);

// attach interrupt to both buttons
  attachInterrupt(digitalPinToInterrupt(gButton1), isrButton1, FALLING);
  attachInterrupt(digitalPinToInterrupt(gButton2), isrButton2, FALLING);

}

void loop() {

// if the interrupt is seen (button 1 is pressed setting flag to 1)
if (gFlag_1){
  gFlag_1 = 0;    // reset flag 
  unsigned long current = millis();   // set current time using millis function
  if (current - gLastTime1 >= gDebounce) {  // if the current time subtract by the last time button was pressed is greater or = to our debounce time
    gCounter1++;    // increment the counter
    gLastTime1 = current;   // set the last pressed time to the current time
  }
}

// if the interrupt is seen (button 2 is pressed setting flag to 1)
if (gFlag_2){
  gFlag_2 = 0;
  unsigned long current = millis();     // set current time using millis function
  if (current - gLastTime2 >= gDebounce) {    // if the current time subtract by the last time button was pressed is greater or = to our debounce time
    gCounter2++;      // increment the counter
    gLastTime2 = current;   // set the last button press time to current time
  }
}
  if (gCounter1 > gCounter2){     // turn on red LED if button 1 counter is greater than button 2 counter
    digitalWrite(gR_LED, HIGH);
    digitalWrite(gG_LED, LOW);
    digitalWrite(gB_LED, LOW);
  }
  else if (gCounter1 == gCounter2) {  // turn on blue LED if both counters are equal
    digitalWrite(gR_LED, LOW);
    digitalWrite(gG_LED, LOW);
    digitalWrite(gB_LED, HIGH);   
  }
  else {
    digitalWrite(gR_LED, LOW);    // green led turns on if counter 2 is greater than counter 1
    digitalWrite(gG_LED, HIGH);
    digitalWrite(gB_LED, LOW);
  }
  
  // print counter values
  Serial.print("Button 1: ");   
  Serial.print(gCounter1);
  Serial.print(" ");
  Serial.print("  Button 2: ");
  Serial.println(gCounter2);

  delay(500);   // delay 500 ms to be able to read output
}


// interrupts created to capture when button1/2 is pressed to set the flag to 1 to intiate the debouncer and counter
void isrButton1() {
  gFlag_1 = 1;
}

void isrButton2() {
  gFlag_2 = 1;
}


