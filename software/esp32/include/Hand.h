#pragma once
#include <Arduino.h>
#include "Finger.h"
#include "FingerMovement.h"

#define FINGER_COUNT 5

/*
 *   Thumb pulley expects 120°, other pulleys expect 180°
 */
#define THUMB_PIN D6
#define THUMB_MAX_OPEN 30
#define THUMB_MAX_CLOSED 150

#define FINGER1_PIN D10
#define FINGER1_MAX_OPEN 10 
#define FINGER1_MAX_CLOSED 180

// Finger 3 (offset 2) servo is mounted on the other side => reverse open/close positions
#define FINGER2_PIN D9
#define FINGER2_MAX_OPEN 180
#define FINGER2_MAX_CLOSED 20

#define FINGER3_PIN D8
#define FINGER3_MAX_OPEN 10
#define FINGER3_MAX_CLOSED 180

// Finger 5 (offset 4) servo is mounted on the other side => reverse open/close positions
#define FINGER4_PIN D7
#define FINGER4_MAX_OPEN 170
#define FINGER4_MAX_CLOSED 35


class Hand {
public:
   Hand();

   Finger *fingers[5]; // 0 is thumb
   void close();
   void close(uint finger);

   void open();
   void open(uint finger);

   bool isStill();
   bool isStill(uint finger);

   void setStep(int step);
   void setStep(int finger, int step);

   void setFinger(uint finger, FingerMovement *fingerMovement);
   void run();
   void run(uint finger);
   void stop();
   void stop(uint finger);
};