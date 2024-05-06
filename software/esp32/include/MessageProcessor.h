
#pragma once

#include <Arduino.h>
#include "Hand.h"
#include "Sequence.h"
#include "HandMovement.h"
#include "HandMovementFactory.h"
#include <BLEDevice.h>


enum MessageType {movement, config};

class MessageProcessor {
public:
   MessageProcessor(Hand *hand);
   void run();
   void processWriteMsg(MessageType type, char *message);
   void processReadMsg(MessageType type, char *message, BLECharacteristic *characteristic);
   void startMovement(char *movementName);
   void calibration();

   Hand *hand = NULL;
   Sequence *seq = NULL;
   HandMovement *handMovement = NULL;
   HandMovementFactory *hmf = NULL;

};