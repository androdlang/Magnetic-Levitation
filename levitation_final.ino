#include <FastLED.h>

#define LED_PIN     8
#define CLK_PIN     9
#define COLOR_ORDER BGR
#define CHIPSET     APA102
#define NUM_LEDS    8
#define BRIGHTNESS  100
#define FRAMES_PER_SECOND 10
CRGB leds[NUM_LEDS];
CRGBPalette16 currentPalette;
TBlendType    currentBlending;
uint8_t startIndex = 0;

int cycle=0;
long lc=0;
byte co=0;
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

  // unsigned char low = ADCL;
  //  unsigned char high = ADCH;
  // return  (high << 8) + low;

  // this is here just for fun
  // it annoys me that AVR GCC wastes cycles on combining ADCL and ADCH
  unsigned int res;

  asm volatile ( 
  "lds %A0, 0x0078 \n\t" // ADCL
  "lds %B0, 0x0079 \n\t" // ADCH
: 
  "=&r" (res) 
    ) ;

  return res; 
  
}

inline void digitalWriteD(unsigned char bit, unsigned char state) {
  if (state) {
    sbi(PORTD, bit);
  } 
  else {
    cbi(PORTD, bit);
  }
}

// disable timer 0 overflow interrupt
void disableMillis() {
  cbi(TIMSK0, TOIE0);
}


void setup() 
{

  // open serial connection, 1,000,000 baud
  Serial.begin(9600);
  delay(300); // sanity delay
  FastLED.addLeds<CHIPSET, LED_PIN,CLK_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness( BRIGHTNESS );
        currentPalette = RainbowColors_p;         
        currentBlending = LINEARBLEND;
  
  // setup 19230.8Hz sampling rate
  // setup Vcc as ref, right aligned output
  analogSetup(3, 1, 0);

  // free running mode
  ADCSRB = ADCSRB & 0xf8;

  // setup pin 3 as output
  pinMode(3,OUTPUT);

  // enable autotrigger
  sbi(ADCSRA, ADATE);


  // start conversion from pin 0
  analogStart(0);

  //disableMillis();
}

void nextLightPattern() {
  for(int j=0;j<NUM_LEDS;j+=1) {
    //fill_solid(leds, NUM_LEDS, CHSV(cycle&1?0:128,255,128)); 
    if(j==(cycle&7)||j==((cycle+4)&7))
      leds[j]=CHSV((co+128)&0xff,255,192); 
    else
      leds[j]=CHSV((co+0)&0xff,255,192); 
  }
  FastLED.show(); // display this frame
  cycle++;
  co+=5;
}
void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
    uint8_t brightness = 100;
    
    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
        colorIndex += random8(20);
    }
}


void nextPalette() {
    startIndex = startIndex + random8(); /* motion speed */
    FillLEDsFromPaletteColors( startIndex);
    FastLED.show();

}
void loop() {
  
  lc++;
  if((lc&0x3ffff)==0) {
    //if(lc&0x400000)
    //  nextLightPattern();
    //else
    
      nextPalette();
  }
  
  unsigned int sensorValue = analogNext();
  if (sensorValue >= 622|| sensorValue < 572) {
    digitalWriteD(3, 0);
  }
  else {
    digitalWriteD(3, 1);
  }
}
