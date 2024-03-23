
#pragma once

#include <Arduino.h>
#include <Servo.h>
#include "FingerMovement.h"


class Finger {
public:   
   Finger(int pin, int maxOpen, int maxClosed, int direction);

   void move(int to);
   void setStep(int step);
   void close();
   void open();
   void run();
   void stop();
   bool isStill();
   void setMovement(FingerMovement *fingerMovement);
   
   Servo myServo;
   int pin = 29;
   int maxOpen = 0; 
   int maxClosed = 180;

   int current = maxOpen;
   int target = maxOpen;
   int frequency = 100;
   int step = DEFAULT_STEP;
   int direction = 1;
};