#pragma once

#include "HandMovement.h"

class HandMovementFactory {
public:

   HandMovementFactory(Hand *hand);

   HandMovement* fist();
   HandMovement* ok();
   HandMovement* one(boolean silent = false);
   HandMovement* two();
   HandMovement* three();
   HandMovement* four();
   HandMovement* five();
   HandMovement* closePinch();
   HandMovement* openPinch();
   HandMovement* idle();
   HandMovement* point();
   
   Hand *hand;
};