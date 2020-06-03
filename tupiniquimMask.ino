/*
Escrito originalmente por Adafruit Industries.  Distribuído por licença BSD.
Modificado e adaptado por Gedeane Kenshima
30-05-2020
*/

#include <Adafruit_NeoPixel.h>

#define N_PIXELS  35  // Quantidade de LEDs na fita
#define MIC_PIN   A9  // OUT sensor de som (microfone)
#define LED_PIN    6  // Pino DIN da Flora
#define DC_OFFSET  0  // DC offset in mic signal - if unusure, leave 0
#define NOISE     10  // Noise/hum/interference in mic signal
#define SAMPLES   60  // Length of buffer for dynamic level adjustment
#define TOP       (N_PIXELS + 2) // Allow dot to go slightly off scale
#define PEAK_FALL 40  // Rate of peak falling dot

byte
  peak      = 0,      // Used for falling dot
  dotCount  = 0,      // Frame counter for delaying dot-falling speed
  volCount  = 0;      // Frame counter for storing past volume data
int
  vol[SAMPLES],       // Collection of prior volume samples
  lvl       = 10,      // Current "dampened" audio level
  minLvlAvg = 0,      // For dynamic adjustment of graph low & high
  maxLvlAvg = 512,
  a;
Adafruit_NeoPixel
  strip = Adafruit_NeoPixel(N_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

void desliga(){
  for(int i=0;i<N_PIXELS;i++){
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    strip.setPixelColor(i, strip.Color(0,0,0)); // Moderately bright green color.
    strip.show(); // This sends the updated pixel color to the hardware.

   }
  //delay(50); // Delay for a period of time (in milliseconds).
}

void sorriso(){
  strip.setPixelColor(7, strip.Color(255,0,255)); // purple color.
  strip.setPixelColor(13, strip.Color(255,0,255)); // purple color.

  strip.setPixelColor(15, strip.Color(255,0,255)); // purple color.
  strip.setPixelColor(19, strip.Color(255,0,255)); // purple color.

  strip.setPixelColor(23, strip.Color(255,0,255)); // purple color.
  strip.setPixelColor(24, strip.Color(255,0,255)); // purple color.
  strip.setPixelColor(25, strip.Color(255,0,255)); // purple color.
  strip.show(); // This sends the updated pixel color to the hardware.
  delay(100); // Delay for a period of time (in milliseconds).
}
void fechada(){
  for(int i=14;i<28;i++){
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    strip.setPixelColor(i, strip.Color(0,150,40)); // Moderately bright cyan color.
    strip.show(); // This sends the updated pixel color to the hardware.
  }
  delay(100); // Delay for a period of time (in milliseconds).
}

void aberta(){
  strip.setPixelColor(2, strip.Color(0,0,255)); // blue color.
  strip.setPixelColor(3, strip.Color(0,0,255)); // blue color.
  strip.setPixelColor(4, strip.Color(0,0,255)); // blue color.

  strip.setPixelColor(8, strip.Color(0,0,255)); // blue color.
  strip.setPixelColor(12, strip.Color(0,0,255)); // blue color.

  strip.setPixelColor(15, strip.Color(0,0,255)); // blue color.
  strip.setPixelColor(19, strip.Color(0,0,255)); // blue color.

  strip.setPixelColor(22, strip.Color(0,0,255)); // blue color.
  strip.setPixelColor(26, strip.Color(0,0,255)); // blue color.

  strip.setPixelColor(30, strip.Color(0,0,255)); // blue color.
  strip.setPixelColor(31, strip.Color(0,0,255)); // blue color.
  strip.setPixelColor(32, strip.Color(0,0,255)); // blue color.
  strip.show(); // This sends the updated pixel color to the hardware.
  delay(100); // Delay for a period of time (in milliseconds).

}


void setup() {

  // This is only needed on 5V Arduinos (Uno, Leonardo, etc.).
  // Connect 3.3V to mic AND TO AREF ON ARDUINO and enable this
  // line.  Audio samples are 'cleaner' at 3.3V.
  // COMMENT OUT THIS LINE FOR 3.3V ARDUINOS (FLORA, ETC.):
//  analogReference(EXTERNAL);
  Serial.begin(9600);
  memset(vol, 0, sizeof(vol));
  strip.begin();
}

void loop() {
  desliga(); // Function off LEDs
  uint8_t  i;
  uint16_t minLvl, maxLvl;
  int      n, height;

  n   = analogRead(MIC_PIN);                        // Raw reading from mic 
  n   = abs(n - 512 - DC_OFFSET); // Center on zero
  n   = (n <= NOISE) ? 0 : (n - NOISE);             // Remove noise/hum
  lvl = ((lvl * 7) + n) >> 3;    // "Dampened" reading (else looks twitchy)

  // Calculate bar height based on dynamic min/max levels (fixed point):
  height = TOP * (lvl - minLvlAvg) / (long)(maxLvlAvg - minLvlAvg);
  //Serial.println(n);
  if(height < 0L)       height = 0;      // Clip output
  else if(height > TOP) height = TOP;
  if(height > peak)     peak   = height; // Keep 'peak' dot at top
Serial.println(n);
if(n> 40 && n<90){
  a=0;
  //sorriso();
}
else if(n>=0 && n<25){
  a=1;
  fechada();
}
else if(n>100 && n<600){
  a=2;
  //aberta();
}

switch(a){
  case 0: sorriso();
  break;

  case 1: fechada();
  break;

  case 2: aberta();
  break;
}
  
// Every few frames, make the peak pixel drop by 1:

    if(++dotCount >= PEAK_FALL) { //fall rate 
      
      if(peak > 0) peak--;
      dotCount = 0;
    }

  vol[volCount] = n;                      // Save sample for dynamic leveling
  if(++volCount >= SAMPLES) volCount = 0; // Advance/rollover sample counter

  // Get volume range of prior frames
  minLvl = maxLvl = vol[0];
  for(i=1; i<SAMPLES; i++) {
    if(vol[i] < minLvl)      minLvl = vol[i];
    else if(vol[i] > maxLvl) maxLvl = vol[i];
  }
  // minLvl and maxLvl indicate the volume range over prior frames, used
  // for vertically scaling the output graph (so it looks interesting
  // regardless of volume level).  If they're too close together though
  // (e.g. at very low volume levels) the graph becomes super coarse
  // and 'jumpy'...so keep some minimum distance between them (this
  // also lets the graph go to zero when no sound is playing):
  if((maxLvl - minLvl) < TOP) maxLvl = minLvl + TOP;
  minLvlAvg = (minLvlAvg * 63 + minLvl) >> 6; // Dampen min/max levels
  maxLvlAvg = (maxLvlAvg * 63 + maxLvl) >> 6; // (fake rolling average)

}

// Input a value 0 to 255 to get a color value.
// The colors are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}
