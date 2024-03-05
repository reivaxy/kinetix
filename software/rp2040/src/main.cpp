#include <Arduino.h>
#include <FastLED.h>
#include "Hand.h"
#include "HandMovementFactory.h"
#include "Sequence.h"

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
Sequence *seq = NULL;

void setup() {
  FastLED.addLeds<WS2812 , DATA_PIN, RGB>(leds, NUM_LEDS); 
  FastLED.setBrightness(5);
  leds[0] = CRGB::Blue;
  FastLED.show();
  Serial.begin(115200);
  Serial.println("started");

  delay(1000);
  leds[0] = CRGB::White;
  FastLED.show();
  delay(500);
  leds[0] = CRGB::Red;
  FastLED.show();
  delay(500);
  leds[0] = CRGB::Green;
  FastLED.show();

  start = millis();
  isClosed = true;
  hand = new Hand();
  hmf = new HandMovementFactory(hand);
  seq = new Sequence(3); // 0 is repeat forever
  seq->addMovement(hmf->fist());
  seq->addMovement(hmf->one());
  seq->addMovement(hmf->two());
  seq->addMovement(hmf->three());
  seq->addMovement(hmf->four());
  seq->addMovement(hmf->idle(), 5000);
  seq->addMovement(hmf->fu());
  seq->addMovement(hmf->openPinch());
  seq->addMovement(hmf->ok());
  seq->addMovement(hmf->pointing());
  seq->addMovement(hmf->closePinch());

  seq->start();

}

void loop() {
  seq->run();
}