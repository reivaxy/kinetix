
#pragma once

#include <Arduino.h>
#include "Hand.h"
#include "Finger.h"
#include "FingerMovement.h"

class HandMovement {
public:  
   HandMovement(Hand *_hand, String _name = "default");
   ~HandMovement();

   void start();
   void run();
   void stop();
   boolean isFinished();
   void printMovement();
   void setFM(uint offset, FingerMovement *fm);

   Hand *hand = NULL;
   String name;
   FingerMovement *fingerMovement[5];

   uint32_t startedAt = 0;
   boolean running = false;
};