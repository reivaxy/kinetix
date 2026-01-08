
#pragma once

#include <Arduino.h>
#include "Hand.h"
#include "Sequence.h"
#include "Settings.h"
#include "HandMovement.h"
#include "HandMovementFactory.h"
#include <BLEDevice.h>



enum MessageType {movement, setting, ota, systemConfig};

class MessageProcessor {
public:
   MessageProcessor(Hand *hand, Settings *settings, Display *display, JsonDocument &systemInfo);
   void run();
   void processWriteMsg(MessageType type, char *message);
   void processReadMsg(MessageType type, BLECharacteristic *characteristic);
   void startMovement(char *movementName);
   void calibration();
   void scratch();
   void come();
   void demo();
   boolean isIdle();

   JsonDocument systemInfo;
   Hand *hand = NULL;
   Display *display = NULL;
   Sequence *seq = NULL;
   HandMovement *handMovement = NULL;
   HandMovementFactory *hmf = NULL;
   Settings *settings = NULL;

};