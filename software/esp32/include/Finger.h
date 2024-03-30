
#pragma once

#include <Arduino.h>
#include <Servo.h>
#include "FingerMovement.h"
#include "CurrentMonitor.h"


class Finger {
public:   
   Finger(int number, int controlPin, int monitorPin, int maxOpen, int maxClosed, int direction);

   void move(int to);
   void setStep(float step);
   void close();
   void open();
   void run();
   void stop();
   bool isStill();
   void setMovement(FingerMovement *fingerMovement);
   
   Servo myServo;

   int number = 0;
   int controlPin = D6;
   CurrentMonitor currentMonitor;
   int maxOpen = 0; 
   int maxClosed = 180;

   float currentPosition = maxOpen;
   int target = maxOpen;
   int frequency = 100;
   float step = DEFAULT_STEP;
   int direction = 1;
};