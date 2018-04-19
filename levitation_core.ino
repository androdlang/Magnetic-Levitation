//(c) 2018 johann@langhofer.net

#define OUT_PIN 3
#define IN_PIN 0
#define THRESHOLD 622
#define OUTOFRANGE THRESHOLD-50

// clear bit and set bit
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif



void analogSetup(unsigned char prescaler, unsigned char reference, unsigned char adjust) {
  //value  prescaler adc clock max freq
  //1 2 8000000 615385.  Hz
  //2 4 4000000 307692.  Hz
  //3 8 2000000 153846.  Hz
  //4 16  1000000 76923.1  Hz
  //5 32  500000  38461.5  Hz
  //6 64  250000  19230.8  Hz
  //7 128 125000  9615.38  Hz

  // reference
  // 0  AREF
  // 1  Vcc
  // 2  reserved
  // 3  1.1V

  // adjust
  // 0  right adjust
  // 1  left adjust

  ADMUX = (ADMUX & 0x1f) | ((reference << 6) | adjust << 5);
  ADCSRA = (ADCSRA & 0xf8) | prescaler;
}

// start conversion
void analogStart(unsigned char pin) {
  ADMUX = (ADMUX & 0xf0) | pin;
  sbi(ADCSRA, ADSC);
}

// waits until another conversion is finished by waiting for
// the interrupt flag to be set
// reads the 10bit result and resets the interrupt flag
unsigned int analogNext() {
  while (!(ADCSRA & _BV(ADIF))) ;

  // reset the flag
  sbi(ADCSRA, ADIF);

  unsigned char low = ADCL;
  unsigned char high = ADCH;
  return  (high << 8) + low;
}

inline void digitalWriteD(unsigned char bit, unsigned char state) {
  if (state) {
    sbi(PORTD, bit);
  } 
  else {
    cbi(PORTD, bit);
  }
}


void setup() 
{

  // open serial connection, 1,000,000 baud
  Serial.begin(9600);
  
  // setup 153846.Hz sampling rate
  // setup Vcc as ref, right aligned output
  analogSetup(3, 1, 0);

  // free running mode
  ADCSRB = ADCSRB & 0xf8;

  // setup pin 3 as output
  pinMode(OUT_PIN,OUTPUT);

  // enable autotrigger
  sbi(ADCSRA, ADATE);


  // start conversion from pin 0
  analogStart(IN_PIN);

}

void loop() {
  
  unsigned int sensorValue = analogNext();
  if (sensorValue >= THRESHOLD|| sensorValue < OUTOFRANGE) {
    digitalWriteD(OUT_PIN, 0);
  }
  else {
    digitalWriteD(OUT_PIN, 1);
  }
}
