
#pragma once

#include <Arduino.h>
#include <Servo.h>
#include "FingerMovement.h"

class Finger {
public:   
   Finger(int number, int controlPin, int maxOpen, int maxClosed, int direction);

   void move(int to);
   void computeTarget(int to);
   void refresh();
   void setStep(float step);
   void setMaxOpen(int max);
   void setMaxClosed(int max);
   void setMax(int maxOpen, int maxClosed);
   void close();
   void open();
   void run();
   void stop();
   bool isStill();
   void setMovement(FingerMovement *fingerMovement);
   void resetMovement();
   
   Servo myServo;

   int number = 0;
   int controlPin = D6;
   int maxOpen = 0; 
   int maxClosed = 180;
   time_t movementStartedAt = 0;
   uint32_t delay = 0;
   float currentPosition = maxOpen;
   int currentNormalizedPosition = 0;
   int target = maxOpen;
   int frequency = 100;
   float step = DEFAULT_STEP;
   int direction = 1;
};