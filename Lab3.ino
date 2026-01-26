// Lab 3 - Lauren Ooghe 
// Using interrupts, have 1 main task running with 2 other tasks running in background

// assign LED pins
# define R_LED 5
# define G_LED 4
# define TIMER_COMPARE_VALUE_1 31249  // see README.md file for breakdown
# define TIMER_COMPARE_VALUE_2 155    // see README.md file for breakdown

// array to print greetings in different languages for fun
const char* greetings[] = {
  "Hello",
  "Hola",
  "Bonjour",
  "Ciao",
  "Hallo",
  "Konnichiwa",
  "Namaste",
  "Zdravstvuyte",
  "Salam",
  "Ni Hao",
  "Annyeong",
  "Hej",
  "Hei",
  "Ahoj",
  "Merhaba",
  "Shalom",
  "Xin Chao",
  "Sawadee",
  "Ola",
  "Jambo",
  "Szia",
  "Halo",
  "Selam"
};

int gButton = 2;  // assign button pin
int gTimer = 0;   // create timer variable

// assign volatile ISR flags (used simultaneously)
volatile unsigned char gFlag_1 = 0;
volatile unsigned char gFlag_2 = 0;

// used to increment timer 2 in task 2
volatile int gCounter_2 = 0;

// used to toggle green LED in task 1
bool gGreenLEDState = LOW;


void setup() {
  Serial.begin(9600);   // begin serial monitoring
  // assign INPUT/OUTPUT
  pinMode(gButton, INPUT);
  pinMode(R_LED, OUTPUT);
  pinMode(G_LED, OUTPUT);

  noInterrupts();   // disable interrupts

  // ***** TASK 1 @ 1 Hz ******
  // reset these to 0 initially to clear any previous
  TCCR1A = 0; 
  TCCR1B = 0;

  OCR1A = TIMER_COMPARE_VALUE_1; // 16-bit compare match register -- sets exact count at which the interrupt is triggered 
                                  // (prescalar)256/16MHz = .000016 --> 0.5s/0.000016 - 1 = 31249
  TCCR1B = (1<<WGM12) | (1<<CS12); // setting to CTC mode by bit masking
  TIMSK1 |= (1<<OCIE1A);  // enable timer compare interrupt (bit mask OCIE1A to enable)

  // ***** TASK 2 @ 10 Hz *****
  TCCR2A = 0;   // reset to 0 to clear
  TCCR2B = 0; 

  OCR2A = TIMER_COMPARE_VALUE_2;  // 8-bit compare match register (prescalar)1024/16MHz = .000064 --> 0.01/0.000064 -1 = 155 
  TCCR2A = (1<<WGM21);            // Time Counter Control Register -- set WGM21 bit to 1 for CTC mode
  TCCR2B = (1<<CS22) | (1<<CS21) | (1<<CS20);   // select 1024 prescalar
  TIMSK2 |= (1<<OCIE2A);          // enable timer compare interrupt
  interrupts();   // enable interrupts

}


void loop() {

  // ********** TASK 1 ***********
  if (gFlag_1){   // if flag1 is 1
    gFlag_1 = 0;  // reset flag to 0
    digitalWrite(G_LED, gGreenLEDState);  // turn LED on
    gGreenLEDState = !gGreenLEDState;     // toggle from LOW to HIGH @ 1 Hz
    gTimer++;   // increment timer every 1s
  }
  // ********** TASK 2 ************
  if (gFlag_2) {    // if flag2 is 1
    gFlag_2 = 0;    // reset flag to 0
    bool button = digitalRead(gButton);   // read button input
    if (button){    // if button is pressed
      digitalWrite(R_LED, HIGH);    // turn red LED on
    }
    else {
      digitalWrite(R_LED, LOW);     // if button not pressed, turn red LED off
  }
  }


// ***** MAIN TASK *****
  if (gTimer % 7 == 0) {  
    int index = random(22);  // randomly select index of array
    Serial.println(greetings[index]); // print random index of array
    gTimer++;
  }
}


// Task 1 interrupt
ISR(TIMER1_COMPA_vect){   
  gFlag_1 = 1;    // once triggered, set flag to 1
}

// Task 2 interrupt
ISR(TIMER2_COMPA_vect) {
  gCounter_2++; // counter increment
                // counter needed to meet 10 Hz requirement. Cannot get to 10 Hz using available Timer2 (only 8 bits), can only go up to 255
  if (gCounter_2 >= 10) { // once counter reaches 10
    gCounter_2 = 0;       // reset to 0
    gFlag_2 = 1;    // set flag to 1
  }
}

