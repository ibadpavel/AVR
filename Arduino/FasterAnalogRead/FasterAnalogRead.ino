#define FASTADC 1

// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#define ARRAYSIZE 700
uint8_t sample_array[ARRAYSIZE];

uint32_t const_flag = 0xABCDEF00;
uint32_t start_millis = 0;  
uint32_t ms = 0;

void setup() {    
#if FASTADC
  sbi(ADCSRA,ADPS2);
  cbi(ADCSRA,ADPS1);
  cbi(ADCSRA,ADPS0);
#endif

  Serial.begin(115200);
  // start_millis = millis();
  // int i;
  // Serial.print("ADCTEST: ");
  // start_millis = millis();
  // for (i = 0; i < 1000; i++) {
  //   ms = analogRead(0);
  //   Serial.write(reinterpret_cast<uint8_t*>(&ms), 2);
  // }

  // Serial.println();
  // Serial.println();
  // Serial.println();  
  // Serial.print(millis() - start_millis) ;
  // Serial.println(" msec (1000 calls)") ;
}

void loop() {
  ms = analogRead(0);
  Serial.write(reinterpret_cast<uint8_t*>(&ms), 2);

  //for (uint16_t i = 0; i < ARRAYSIZE; i++) {
  //  sample_array[i] = static_cast<uint8_t>(analogRead(0));
  //}
 
  //ms = millis() - start_millis; 
  //Serial.write(reinterpret_cast<uint8_t*>(&const_flag), 4);
  //Serial.write(reinterpret_cast<uint8_t*>(&ms), 4);
  
  //for (short i = 0; i < ARRAYSIZE; i++) {    
  //  Serial.write(sample_array[i]);
  //  //Serial.println(sample_array[i]);
  //}
  //Serial.println();
}