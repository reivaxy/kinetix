
#pragma once

#include <Arduino.h>
#include "Hand.h"
#include "Sequence.h"
#include "Settings.h"
#include "HandMovement.h"
#include "HandMovementFactory.h"
#include <BLEDevice.h>

// Forward declaration
class BtServer;

enum MessageType {movement, setting, ota, systemConfig, positions, password};

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
   Sequence* demo();
   boolean isIdle();
   
   void setBtServer(BtServer *btServer);

   JsonDocument systemInfo;
   Hand *hand = NULL;
   Display *display = NULL;
   Sequence *seq = NULL;
   HandMovement *handMovement = NULL;
   HandMovementFactory *hmf = NULL;
   Settings *settings = NULL;
   BtServer *btServer = NULL;

};