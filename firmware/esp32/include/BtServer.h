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
#define SYSTEM_CHARACTERISTIC_UUID "b2a49d41-a2ac-48c3-b6c8-cfd05640654e"
#define CONFIG_CHARACTERISTIC_UUID "68b788da-819b-4feb-b478-8d237ef29f5f"
#define POSITIONS_CHARACTERISTIC_UUID "4a7e0a3f-8c45-4b6d-9c2a-f3d1e5b7a9c2"
#define OTA_CHARACTERISTIC_UUID "3168e56f-6ea1-420d-98f8-08a3b34afc9b"
#define PASSWORD_CHARACTERISTIC_UUID "7c4a2e1f-5b9a-4d8e-9c3b-2f8a1e5c6d7a"

#define MAX_MESSAGE_SIZE 150
#define PASSWORD_TIMEOUT_MS 30000  // 30 seconds

class BtServer {
public:   
   BtServer(MessageProcessor *messageProcessor, Display *display);

   MessageProcessor *messageProcessor;
   Display *display = NULL;
   
   // Authentication state management
   void setClientAuthenticated(bool authenticated);
   bool isClientAuthenticated();
   void setPasswordTimeout(uint32_t timeoutMs);
   void checkPasswordTimeout();
   void disableAllCharacteristics();
   void enableAllCharacteristics();
   void resetAuthenticationState();

private:
   bool clientAuthenticated = false;
   uint32_t authenticationTimestamp = 0;
   uint32_t passwordTimeoutMs = PASSWORD_TIMEOUT_MS;
   BLEServer* pServer = nullptr;
   uint16_t clientConnId = 0;
   BLECharacteristic* pMovementCharacteristic = nullptr;
   BLECharacteristic* pSystemCharacteristic = nullptr;
   BLECharacteristic* pConfigCharacteristic = nullptr;
   BLECharacteristic* pPositionsCharacteristic = nullptr;
   
friend class MyServerCallback;
};