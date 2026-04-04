#pragma once
#include <Arduino.h>
#include <driver/adc.h>

#include "Finger.h"
#include "FingerMovement.h"

#include "Settings.h"
#include "OptionalDisplay.h"

#define FINGER_COUNT 5

/*
 *   Thumb pulley expects 120°, other pulleys expect 180°
 */
#define THUMB_CTRL_PIN D6

#define FINGER1_CTRL_PIN D10

#define FINGER2_CTRL_PIN D9

#define FINGER3_CTRL_PIN D8

#define FINGER4_CTRL_PIN D7

/* Fingers 3 and 5 servos are mounted in oposite direction so their min and max are reversed compared to other fingers. */
/* Left hand and right hand are symetrical so min and max need to be reversed */
/* servos homing is supposed to be done before wiring, when servos can go full range from 0 to 180° */


#ifdef LEFT_HAND
   #define SERVO_CALIBRATION_OPEN 180
   #define SERVO_CALIBRATION_CLOSED 0

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
   #define SERVO_CALIBRATION_OPEN 0
   #define SERVO_CALIBRATION_CLOSED 180

   #define THUMB_MAX_OPEN 30
   #define THUMB_MAX_CLOSED 180

   #define FINGER1_MAX_OPEN 10 
   #define FINGER1_MAX_CLOSED 180

   #define FINGER2_MAX_OPEN 180
   #define FINGER2_MAX_CLOSED 0

   #define FINGER3_MAX_OPEN 0
   #define FINGER3_MAX_CLOSED 180

   #define FINGER4_MAX_OPEN 180
   #define FINGER4_MAX_CLOSED 35
#endif


class Hand {
public:
   Hand(Settings *settings);

   Finger *fingers[5]; // 0 is thumb

   void close();
   void close(uint finger);
   void setCalibration(boolean);

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
   void moveRelative(int to);
   Settings *settings
};