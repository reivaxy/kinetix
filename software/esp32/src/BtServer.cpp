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


  void onWrite(BLECharacteristic *characteristic) override {
    log_i("Kinetix received a write request"); 
    char message[MAX_MESSAGE_SIZE];
    size_t len = min((int)characteristic->getLength(), MAX_MESSAGE_SIZE);
    strncpy(message, (char *)(characteristic)->getData(), len);
    message[len] = 0;
    if (messageProcessor != NULL) {
      messageProcessor->processWriteMsg(type, message);
    }
  }
  
    void onRead(BLECharacteristic *characteristic) override {
    log_i("Kinetix received a read request"); 
    char message[MAX_MESSAGE_SIZE];
    size_t len = min((int)characteristic->getLength(), MAX_MESSAGE_SIZE);
    strncpy(message, (char *)(characteristic)->getData(), len);
    message[len] = 0;
    if (messageProcessor != NULL) {
      messageProcessor->processReadMsg(type, message, characteristic);
    }
  }


};

class MyServerCallback : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    log_i("Client connected.");
  }

  void onDisconnect(BLEServer* pServer) {
    log_i("Client disconnected");
    // Need to restart advertising to be able to reconnect
    pServer->getAdvertising()->start();
  }
};

BtServer::BtServer(MessageProcessor *_messageProcessor) {
  messageProcessor = _messageProcessor;
  BLEDevice::init("Kinetix");
  BLEServer *pServer = BLEDevice::createServer();

  pServer->setCallbacks(new MyServerCallback());
   
  BLEService *pService = pServer->createService(SERVICE_UUID);
  
  // TODO: create different characteristic for movements, config, calibration, ... ?
  BLECharacteristic *pMovementCharacteristic = pService->createCharacteristic(
                                         MOVEMENT_CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  BLECharacteristic *pConfigCharacteristic = pService->createCharacteristic(
                                         CONFIG_CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ
                                       );

  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  pServer->getAdvertising()->start();
  pMovementCharacteristic->setCallbacks(new CharacteristicCallBack(movement, messageProcessor));
  pConfigCharacteristic->setCallbacks(new CharacteristicCallBack(config, messageProcessor));
  Serial.println("Kinetix now available for BT connection.");
}
