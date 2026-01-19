// Lab 2 - Q4 - Lauren Ooghe
// Upon pressing the button, an interrupt will run that does arithemtic and uses arrayX values to save into arrayY


#define N 10    // define array size

int gArrayX[N] = {2, 3, 7, 1, 9, 3, 8, 9, 4, 5};    // create array x with 10 random integers
int gArrayY[N] = {};  // create array y, empty set

const int gButton = 2;  // assign button pin 

volatile unsigned char gFlag = 0;   // interrupt flag, once set to 1, interrupt routine will run 

void setup() {

  Serial.begin(9600);   // begin serial monitoring
  
  pinMode(gButton, INPUT);  // set button pin to INPUT

  attachInterrupt(digitalPinToInterrupt(gButton), isrButton, FALLING);   // attach an interrupt to the button so that it will run once pressed
}

void loop() {
  // if the button is pressed, the ISR will set the gFlag to 1 and run this code
  if (gFlag ==1) {
    for (int i = 0; i < 10; i++){   // loop through all elements in array x to complete the arithmetic operations
      gArrayX[-1] = 0;      // needed for the first iteration of loop, since index -1 is out of the array and points to memory location instead
      gArrayY[i] = 2 * gArrayX[i] - gArrayX[i-1];   // computing y[n] = 2 * x[n] - x[n-1] -- saving into array Y
      
    }
    for (int i = 0; i < 10; i++){   // print all elements that were saved into array Y
      Serial.println(gArrayY[i]);
    }
  }
  gFlag = 0;  // reset the flag to 0
}

// interrupt function, called when button is pressed and sets the flag to 1 to run the code
 void isrButton() {
  gFlag = 1;
 }