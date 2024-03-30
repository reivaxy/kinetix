#pragma once

#include <Arduino.h>

#define DEFAULT_STEP 1


class FingerMovement {
public:  
   FingerMovement(int relativeTargetPosition, uint32_t startDelay, float step);
   FingerMovement(int relativeTargetPosition, uint32_t startDelay);
   FingerMovement(int relativeTargetPosition);

   int relativeTargetPosition = 0; // Relative target position: 0 fully open, 10 = 100% fully closed
   uint32_t startDelay = 0;
   float step = DEFAULT_STEP;

};