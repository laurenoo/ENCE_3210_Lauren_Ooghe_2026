
#define TIMER_COMPARE_VALUE 5000 //64kHz

const int gDAC_PIN[10] = {4, 5, 6, 7, 8, 9, 10, 11, 12, 13};

//uint16_t gDAC_value;

const int SINE_SAMPLES = 256;   // points per cycle
uint16_t sineTable[SINE_SAMPLES];

uint16_t gIndex = 0;

void setup() {

  // Init DAC GPIO
  for(int i=0; i<10; i++){
    pinMode(gDAC_PIN[i], OUTPUT);
  }

  // Generate sine lookup table (0–1023)
  for (int i = 0; i < SINE_SAMPLES; i++) {
    float angle = 2.0 * PI * i / SINE_SAMPLES;
    sineTable[i] = (uint16_t)(512 + 511 * sin(angle));
  }

  // Init Timer1 (16bit)
  // Speed of Timer1 = 16MHz/1 = 16MHz
  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  OCR1A = TIMER_COMPARE_VALUE; // 16MHz/1
  TCCR1B |= (1<<WGM12); // CTC Mode
  // Start Timer by setting the prescaler
  TCCR1B |= (1<<CS10); // no prescaler
  TIMSK1 |= (1<<OCIE1A); // Enable timer compare interrupt
  interrupts();
}

void loop() {
}

ISR(TIMER1_COMPA_vect){
  /*
  // Sawtooth Waveform
  outputDAC(gIndex);
  gIndex = (gIndex + 1) % 1024;
  */

  // Sine Wave
  outputDAC(sineTable[gIndex]);
  gIndex = (gIndex + 1) % SINE_SAMPLES;
  
}

void outputDAC(uint16_t value){
  // Faster output
  PORTD = (PORTD & 0x0F) | ((value & 0x0F) << 4);
  PORTB = (PORTB & 0xC0) | (value >> 4);
}
