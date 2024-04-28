#include "BtServer.h"


class CharacteristicCallBack : public BLECharacteristicCallbacks
{
public:
  MessageProcessor *messageProcessor;
  MessageType type;

  CharacteristicCallBack(MessageType type, MessageProcessor *_messageProcessor) {
    this->messageProcessor = _messageProcessor;
    this->type = type;
  }

  //This method not called
  void onWrite(BLECharacteristic *characteristic) override
  {
    log_i("Kinetix received a message"); 
    char message[MAX_MESSAGE_SIZE];
    size_t len = min((int)characteristic->getLength(), MAX_MESSAGE_SIZE);
    strncpy(message, (char *)(characteristic)->getData(), len);
    message[len] = 0;
    if (messageProcessor != NULL) {
      messageProcessor->processMessage(type, message);
    }
  }

};

BtServer::BtServer(MessageProcessor *_messageProcessor) {
  messageProcessor = _messageProcessor;
  BLEDevice::init("Kinetix");
  BLEServer *pServer = BLEDevice::createServer();
   
  BLEService *pService = pServer->createService(SERVICE_UUID);
  
  // TODO: create different characteristic for movements, config, calibration, ... ?
  BLECharacteristic *pMovementCharacteristic = pService->createCharacteristic(
                                         MOVEMENT_CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pService->start();
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Kinetix now available for BT connection.");
  pMovementCharacteristic->setCallbacks(new CharacteristicCallBack(movement, messageProcessor));
}
