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

#define FINGER1_CTRL_PIN D10
#define FINGER1_MONITOR_PIN A1

#define FINGER2_CTRL_PIN D9
#define FINGER2_MONITOR_PIN A2

#define FINGER3_CTRL_PIN D8
#define FINGER3_MONITOR_PIN A3

#define FINGER4_CTRL_PIN D7
#define FINGER4_MONITOR_PIN A4

/* Fingers 3 and 5 servos are mounted in oposite direction so their min and max are reversed compared to other fingers. */
/* Left hand and right hand are symetrical so min and max need to be reversed */
/* servos homing is supposed to be done before wiring, when servos can go full range from 0 to 180° */

#ifdef HOME_SERVOS
   // Settings to set all servos to home position for 4s then opposite for 1s.
   // cables must be on top of pulley when in home position.
   #ifdef LEFT_HAND
      #define THUMB_MAX_OPEN 180
      #define THUMB_MAX_CLOSED 0

      #define FINGER1_MAX_OPEN 180 
      #define FINGER1_MAX_CLOSED 0

      #define FINGER2_MAX_OPEN 0
      #define FINGER2_MAX_CLOSED 180

      #define FINGER3_MAX_OPEN 180
      #define FINGER3_MAX_CLOSED 0

      #define FINGER4_MAX_OPEN 0
      #define FINGER4_MAX_CLOSED 180
   #else
      #define THUMB_MAX_OPEN 0
      #define THUMB_MAX_CLOSED 180

      #define FINGER1_MAX_OPEN 0 
      #define FINGER1_MAX_CLOSED 180

      #define FINGER2_MAX_OPEN 180
      #define FINGER2_MAX_CLOSED 0

      #define FINGER3_MAX_OPEN 0
      #define FINGER3_MAX_CLOSED 180

      #define FINGER4_MAX_OPEN 180
      #define FINGER4_MAX_CLOSED 0
   #endif
#else
   #ifdef LEFT_HAND
      #define THUMB_MAX_OPEN 170
      #define THUMB_MAX_CLOSED 30

      #define FINGER1_MAX_OPEN 180 
      #define FINGER1_MAX_CLOSED 10

      #define FINGER2_MAX_OPEN 20
      #define FINGER2_MAX_CLOSED 180

      #define FINGER3_MAX_OPEN 180
      #define FINGER3_MAX_CLOSED 10

      #define FINGER4_MAX_OPEN 15
      #define FINGER4_MAX_CLOSED 170
   #else
      #define THUMB_MAX_OPEN 0
      #define THUMB_MAX_CLOSED 110

      #define FINGER1_MAX_OPEN 10 
      #define FINGER1_MAX_CLOSED 180

      #define FINGER2_MAX_OPEN 180
      #define FINGER2_MAX_CLOSED 20

      #define FINGER3_MAX_OPEN 0
      #define FINGER3_MAX_CLOSED 170

      #define FINGER4_MAX_OPEN 170
      #define FINGER4_MAX_CLOSED 35
   #endif
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