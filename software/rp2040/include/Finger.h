
#pragma once

#include <Arduino.h>
#include "RP2040_PWM.h"
#include "FingerMovement.h"


class Finger {
public:   
   Finger(int pin, float maxOpen, float maxClosed);
   void setStep(float step);

   void move(float to);
   void close();
   void open();
   void run();
   void stop();
   bool isStill();
   void setMovement(FingerMovement *fingerMovement);
   
   RP2040_PWM* PWMInstance;
   int pin = 29;
   float maxOpen = 8.0;
   float maxClosed = 18.0;

   float current = maxOpen;
   float target = maxOpen;
   float frequency = 100;
   float step = DEFAULT_STEP;
};