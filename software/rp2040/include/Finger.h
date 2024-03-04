
#pragma once

#include <Arduino.h>
#include "RP2040_PWM.h"

class Finger {
public:   
   Finger(int pin, float maxOpen, float maxClosed);
   void setStep(float step);

   void move(float to);
   void close();
   void open();
   void run();
   bool isStill();
   
   RP2040_PWM* PWMInstance;
   int pin = 29;
   float maxOpen = 8.0;
   float maxClosed = 18.0;

   float current = maxOpen;
   float target = maxOpen;
   float frequency = 100;
   float step = 0.0030;
};