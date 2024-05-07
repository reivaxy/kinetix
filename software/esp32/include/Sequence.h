
#pragma once

#include "HandMovement.h"
#define MAX_MOVEMENTS 50

class Sequence {
public:
   Sequence(uint8_t _repeatCount = 1);
   ~Sequence();

   void addMovement(HandMovement* movement, uint32_t movementDuration = 2000);
   void start(uint8_t start = 0);
   void stop();
   void run();

   HandMovement *movements[MAX_MOVEMENTS];
   uint8_t movementCount = 0;
   uint32_t durations[MAX_MOVEMENTS];
   uint32_t previousMovementStarteddAt = 0;

   uint8_t repeatCount = 1; // 0 means repeat forever
   boolean running = false;
   uint8_t current = 0;
   uint8_t loopCount = 0;

};
