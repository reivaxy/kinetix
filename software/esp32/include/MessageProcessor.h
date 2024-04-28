
#pragma once

#include <Arduino.h>
#include "Hand.h"
#include "HandMovement.h"
#include "HandMovementFactory.h"

enum MessageType {movement, config};

class MessageProcessor {
public:
   MessageProcessor(Hand *hand);
   void run();
   void processMessage(MessageType type, char *message);
   void startMovement(char *movementName);

   Hand *hand;
   HandMovement *handMovement;
   HandMovementFactory *hmf;

};