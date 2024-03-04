#pragma once
#include <Arduino.h>
#include "Finger.h"

/*
 *   6, 28 is 180°, but thumb pulley expects 120°
 */

#define THUMB_PIN 0
#define THUMB_MAX_OPEN 6.0
#define THUMB_MAX_CLOSED 18.0

#define FINGER2_PIN 1
#define FINGER2_MAX_OPEN 6.0
#define FINGER2_MAX_CLOSED 28.0

// Finger 3 (offset 2) servo is mounted on the other side => reverse commands
#define FINGER3_PIN 2
#define FINGER3_MAX_OPEN 28.0
#define FINGER3_MAX_CLOSED 9.0

#define FINGER4_PIN 3
#define FINGER4_MAX_OPEN 6.0
#define FINGER4_MAX_CLOSED 28.0

// Finger 5 (offset 4) servo is mounted on the other side => reverse commands
#define FINGER5_PIN 4
#define FINGER5_MAX_OPEN 26.0
#define FINGER5_MAX_CLOSED 10.0


class Hand {
public:
   Hand();

   Finger *fingers[5]; // 0 is thumb
   void close();
   void close(int finger);

   void open();
   void open(int finger);

   bool isStill();
   bool isStill(int finger);

   void setStep(float step);
   void setStep(int finger, float step);

   void run();
};