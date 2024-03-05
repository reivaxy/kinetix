
#pragma once

#include <Arduino.h>
#include "Hand.h"
#include "Finger.h"
#include "FingerMovement.h"

class HandMovement {
public:  
   HandMovement(Hand *_hand);
   void start();
   void run();
   void stop();
   boolean isFinished();

   Hand *hand = NULL;
   FingerMovement *fingerMovement[5];

   uint32_t startedAt = 0;
   boolean running = false;
};