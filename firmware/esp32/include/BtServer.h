#pragma once

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "MessageProcessor.h"
#include <BLE2902.h>
#include <BLEDescriptor.h>
#include "OtaWifiUploadServer.h"

#define SERVICE_UUID        "89d60870-9908-4472-8f8c-e5b3e6573cd1"
#define MOVEMENT_CHARACTERISTIC_UUID "39dea685-a63e-44b2-8819-9a202581f8fe"
#define SYSTEM_CHARACTERISTIC_UUID "68b788da-819b-4feb-b478-8d237ef29f5f"
#define CONFIG_CHARACTERISTIC_UUID "b2a49d41-a2ac-48c3-b6c8-cfd05640654e"
#define OTA_CHARACTERISTIC_UUID "3168e56f-6ea1-420d-98f8-08a3b34afc9b"

#define MAX_MESSAGE_SIZE 100

class BtServer {
public:   
   BtServer(MessageProcessor *messageProcessor);

   MessageProcessor *messageProcessor;
};