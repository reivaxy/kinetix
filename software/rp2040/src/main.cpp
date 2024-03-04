#include <Arduino.h>
#include <FastLED.h>
#include "Hand.h"

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
int count = 0;

void setup() {
  // Serial.begin(115200);
  // while (!Serial && millis() < 10000UL);
  // Serial.println("started");
  FastLED.addLeds<WS2812 , DATA_PIN, RGB>(leds, NUM_LEDS); 
  leds[0] = CRGB::Blue;
  FastLED.show();
  delay(500);
  leds[0] = CRGB::White;
  FastLED.show();
  delay(500);
  leds[0] = CRGB::Red;
  FastLED.show();
  delay(500);

  start = millis();
  isClosed = true;
  hand = new Hand();
  // Serial.println("Hand ready");
  hand->open(); 
  hand->close(finger); 
}

void loop()
{
  if(millis() - start > 300) {
    start = millis();
    if (isClosed) {
      hand->open(finger);
      isClosed = false;
      leds[0] = CRGB::Green;
      FastLED.show();
      if (count % 2 == 0) {
        finger ++;
        if (finger == 5) {
          finger = 0;
        }
      }
      count ++;
    } else {
      hand->close(finger);
      isClosed = true;
      leds[0] = CRGB::Red;
      FastLED.show();
      count++;
    }
  }

  hand->run();
}