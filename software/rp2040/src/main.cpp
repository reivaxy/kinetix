#include <Arduino.h>
#include <FastLED.h>
#include "Hand.h"
#include "HandMovementFactory.h"

// How many leds in your strip?
#define NUM_LEDS 1
// Define the array of leds
CRGB leds[NUM_LEDS];
// For led chips like WS2812, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
// Clock pin only needed for SPI based chipsets when not using hardware SPI
#define DATA_PIN 16
#define USE_WS2812SERIAL 1


Hand *hand = NULL;

int start = 0;
int finger = 0;
bool isClosed = true;

HandMovementFactory *hmf = NULL;
HandMovement *hm = NULL;
uint count = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("started");
  FastLED.addLeds<WS2812 , DATA_PIN, RGB>(leds, NUM_LEDS); 
  FastLED.setBrightness(5);

  leds[0] = CRGB::Blue;
  FastLED.show();
  delay(1000);
  leds[0] = CRGB::White;
  FastLED.show();
  delay(500);
  leds[0] = CRGB::Red;
  FastLED.show();
  delay(500);

  start = millis();
  isClosed = true;
  hand = new Hand();
  hmf = new HandMovementFactory(hand);
  leds[0] = CRGB::Green;
  FastLED.show();

}

void loop() {
  uint32_t elapsed = millis() - start;
  if (elapsed > 3000) {
    delete hm;
    switch(count) {
     case 0:
      hm = hmf->one();
      hm->start();
      leds[0] = CRGB::DarkBlue;
      break;
     case 1:
      hm = hmf->two();
      hm->start();
      leds[0] = CRGB::DarkOrange;
      break;
     case 2:
      hm = hmf->three();
      hm->start();
      leds[0] = CRGB::Yellow;
      break;
     case 3:
      hm = hmf->four();
      hm->start();
      leds[0] = CRGB::DeepPink;
      break;
     case 4:
      hm = hmf->idle();
      hm->start();
      leds[0] = CRGB::MediumPurple;
      break;
    case 5:  
      hm = hmf->openPinch();
      hm->start();
      leds[0] = CRGB::Red;
      count = -1;
      break;
    }
    FastLED.show();
    start = millis();
    count ++;    
  }

  if (hm != NULL) {
    hm->run();
  }

}