
#pragma once

#include <Arduino.h>
#include "Hand.h"
#include "Finger.h"
#include "FingerMovement.h"

class HandMovement {
public:  
   HandMovement(Hand *_hand, String _name = "default");
   void start();
   void run();
   void stop();
   boolean isFinished();
   void printMovement();

   Hand *hand = NULL;
   String name;
   FingerMovement *fingerMovement[5];

   uint32_t startedAt = 0;
   boolean running = false;
};