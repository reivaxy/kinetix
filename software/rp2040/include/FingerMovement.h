#pragma once

#include <Arduino.h>

#define DEFAULT_STEP 0.0045


class FingerMovement {
public:  
   FingerMovement(float relativeTargetPosition, uint32_t startDelay, float step);
   FingerMovement(float relativeTargetPosition, uint32_t startDelay);
   FingerMovement(float relativeTargetPosition);

   float relativeTargetPosition = 0; // Relative target position: 0 fully open, 10 = 100% fully closed
   uint32_t startDelay = 0;
   float step = DEFAULT_STEP;

};