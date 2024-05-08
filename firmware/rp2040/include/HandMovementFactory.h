#pragma once

#include "HandMovement.h"

class HandMovementFactory {
public:

   HandMovementFactory(Hand *hand);

   HandMovement* fist();
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
   
   Hand *hand;
};