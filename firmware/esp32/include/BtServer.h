#pragma once

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "MessageProcessor.h"

#define SERVICE_UUID        "89d60870-9908-4472-8f8c-e5b3e6573cd1"
#define MOVEMENT_CHARACTERISTIC_UUID "39dea685-a63e-44b2-8819-9a202581f8fe"
#define CONFIG_CHARACTERISTIC_UUID "b2a49d41-a2ac-48c3-b6c8-cfd05640654e"

#define MAX_MESSAGE_SIZE 100

class BtServer {
public:   
   BtServer(MessageProcessor *messageProcessor);

   MessageProcessor *messageProcessor;
};