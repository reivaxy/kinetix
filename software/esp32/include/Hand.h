#pragma once
#include <Arduino.h>
#include <driver/adc.h>

#include "Finger.h"
#include "FingerMovement.h"

#define FINGER_COUNT 5

/*
 *   Thumb pulley expects 120°, other pulleys expect 180°
 */
#define THUMB_CTRL_PIN D6
#define THUMB_MONITOR_PIN A0
#ifdef LEFT_HAND
#define THUMB_MAX_OPEN 170
#define THUMB_MAX_CLOSED 30
#else
#define THUMB_MAX_OPEN 30
#define THUMB_MAX_CLOSED 150
#endif

#define FINGER1_CTRL_PIN D10
#define FINGER1_MONITOR_PIN A1
#ifdef LEFT_HAND
#define FINGER1_MAX_OPEN 180 
#define FINGER1_MAX_CLOSED 10
#else
#define FINGER1_MAX_OPEN 10 
#define FINGER1_MAX_CLOSED 180
#endif

// Finger 3 (offset 2) servo is mounted on the other side => reverse open/close positions
#define FINGER2_CTRL_PIN D9
#define FINGER2_MONITOR_PIN A2
#ifdef LEFT_HAND
#define FINGER2_MAX_OPEN 20
#define FINGER2_MAX_CLOSED 180
#else
#define FINGER2_MAX_OPEN 180
#define FINGER2_MAX_CLOSED 20
#endif

#define FINGER3_CTRL_PIN D8
#define FINGER3_MONITOR_PIN A3
#ifdef LEFT_HAND
#define FINGER3_MAX_OPEN 180
#define FINGER3_MAX_CLOSED 10
#else
#define FINGER3_MAX_OPEN 10
#define FINGER3_MAX_CLOSED 180
#endif

// Finger 5 (offset 4) servo is mounted on the other side => reverse open/close positions
#define FINGER4_CTRL_PIN D7
#define FINGER4_MONITOR_PIN A4
#ifdef LEFT_HAND
#define FINGER4_MAX_OPEN 15
#define FINGER4_MAX_CLOSED 170
#else
#define FINGER4_MAX_OPEN 170
#define FINGER4_MAX_CLOSED 35
#endif


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

   void setStep(float step);
   void setStep(int finger, float step);

   void setFinger(uint finger, FingerMovement *fingerMovement);
   void run();
   void run(uint finger);
   void stop();
   void stop(uint finger);
};