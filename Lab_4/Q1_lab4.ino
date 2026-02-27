// simulate in software a 14-bit SAR and test the conversion for the following
// analog voltage levels: 0.42V, 0.83V, 1.65V, and 2.752V with a reference voltage of 3V.

// *** Changes made: bitsize to 14, Vref to 3.0 V, Vref should not be changed in the loop (as it is a reference voltage)
// ** Did no get to make all changes

#define gBitsize 14  // change bit from 12 to 14-bit based on sys design

float Vref = 3.0;
float testVoltage[] = {0.42, 0.83, 1.65, 2.752};
float Vin = 3.3;
float thresh;
float quantized = 0;
int count;
int bitval;
int bits[gBitsize];



void setup() {
  Serial.begin(115200);

  for (int i = 0; i < 4; i++){
    float Vin = testVoltage[i];
    uint16_t code = 0;

    for (int bit = gBitsize - 1; bit >= 0; bit--){
      uint16_t trial = code | (1UL << gBitsize);
      float Vdac = (trial * Vref) / (1UL << gBitsize);

      if (Vdac <= Vin) code = trial;
    }

    float Vq = (code * Vref) / (1UL << gBitsize);

    Serial.print("Vin = ");
    Serial.print(Vin, 3);
    Serial.print (" code= ");
    Serial.print(code);
    Serial.print(" bits = ");
    for (int b = gBitsize-1; b>=0; b--) Serial.print((code >> b) & 1);
    Serial.print("  Vq = "); Serial.println(Vq, 6);
  }
  }


void loop() {

}
