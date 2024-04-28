#pragma once

#include "HandMovement.h"

class HandMovementFactory {
public:

   HandMovementFactory(Hand *hand);
   
   HandMovement* fist();
   HandMovement* half();
   HandMovement* ok();
   HandMovement* one(String name = "One");
   HandMovement* two();
   HandMovement* three();
   HandMovement* four();
   HandMovement* five();
   HandMovement* closePinch();
   HandMovement* openPinch();
   HandMovement* idle();
   HandMovement* pointing();
   HandMovement* fu();
   HandMovement* closeFingerBy(int finger, int closedQuantity);
   HandMovement* getByName(char* movementName);
   
   Hand *hand;
};