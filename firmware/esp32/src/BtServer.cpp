#include "BtServer.h"

#include <BLE2902.h>
#include <BLEDescriptor.h>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "OtaWifiUploadServer.h"

// OTA server singleton (simple; you can turn this into a BtServer member later)
static constexpr uint16_t OTA_PORT = 8080;
static OtaWifiUploadServer gOta(OTA_PORT, nullptr);

// -------------------- BLE TX queue (notify from a safe task) --------------------

// Keep messages bounded (BLE notify MTU often ~20..185 depending on negotiation)
static constexpr size_t BLE_TX_MAX = 180;

struct BleTxMsg {
  char data[BLE_TX_MAX];
};

static QueueHandle_t gBleTxQueue = nullptr;
static BLECharacteristic* gOtaCharForNotify = nullptr;

static void bleTxTask(void* arg) {
  (void)arg;
  BleTxMsg msg{};
  while (true) {
    if (gBleTxQueue && xQueueReceive(gBleTxQueue, &msg, portMAX_DELAY) == pdTRUE) {
      if (gOtaCharForNotify) {
        size_t n = strnlen(msg.data, BLE_TX_MAX);
        gOtaCharForNotify->setValue((uint8_t*)msg.data, n);
        // notify() from this task, NOT from BTC_TASK / BLE callback
        gOtaCharForNotify->notify();
      }
    }
  }
}

static void bleEnqueueNotify(const String& s) {
  if (!gBleTxQueue) return;

  BleTxMsg msg{};
  size_t n = s.length();
  if (n >= BLE_TX_MAX) n = BLE_TX_MAX - 1;
  memcpy(msg.data, s.c_str(), n);
  msg.data[n] = 0;

  // Non-blocking enqueue (drop if full)
  (void)xQueueSend(gBleTxQueue, &msg, 0);
}

// -------------------- BLE callbacks --------------------

class CharacteristicCallBack : public BLECharacteristicCallbacks {
public:
  MessageProcessor* messageProcessor;
  MessageType type;

  CharacteristicCallBack(MessageType type, MessageProcessor* _messageProcessor) {
    this->messageProcessor = _messageProcessor;
    this->type = type;
  }

  void onWrite(BLECharacteristic* characteristic) override {
    log_i("Kinetix received a write request");

    char message[MAX_MESSAGE_SIZE];
    size_t len = min((int)characteristic->getLength(), MAX_MESSAGE_SIZE - 1);
    strncpy(message, (char*)characteristic->getData(), len);
    message[len] = 0;

    // Intercept OTA commands on the OTA channel.
    // IMPORTANT: do NOT call notify() from inside this BLE callback.
    if (type == ota) {
      log_i("Processing OTA command");

      // Reply via queue; notify will happen in bleTxTask()
      gOta.setReply([](const String& s) {
        bleEnqueueNotify(s);
      });

      if (gOta.handleCommand(String(message))) {
        return; // do NOT forward OTA commands to MessageProcessor
      }
    }

    if (messageProcessor != NULL) {
      messageProcessor->processWriteMsg(type, message);
    }
  }

  void onRead(BLECharacteristic* characteristic) override {
    log_i("Kinetix received a read request");
    if (messageProcessor != NULL) {
      messageProcessor->processReadMsg(type, characteristic);
    }
  }
};

class MyServerCallback : public BLEServerCallbacks {
  Display *display;
  
public:
  MyServerCallback(Display *display) : display(display) {
    this->display = display;
  }

private:

  void onConnect(BLEServer* pServer) override {
    log_i("Client connected.");
    display->setLine(1, "BT Connected");
  }
  
  void onDisconnect(BLEServer* pServer) override {
    log_i("Client disconnected");
    display->setLine(1, "BT Disconnected");
    // Need to restart advertising to be able to reconnect
    pServer->getAdvertising()->start();
  }
};

BtServer::BtServer(MessageProcessor* _messageProcessor, Display *display) {
  messageProcessor = _messageProcessor;
  this->display = display;

  BLEDevice::init("KinetiX");
  BLEServer* pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallback(display));

  BLEService* pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic* pMovementCharacteristic =
    pService->createCharacteristic(
      MOVEMENT_CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_WRITE
    );

  BLECharacteristic* pSystemCharacteristic =
    pService->createCharacteristic(
      SYSTEM_CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ
    );

  BLECharacteristic* pConfigCharacteristic =
    pService->createCharacteristic(
      CONFIG_CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ
    );

  // OTA characteristic (WRITE/READ/NOTIFY)
  BLECharacteristic* pOtaCharacteristic =
    pService->createCharacteristic(
      OTA_CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_WRITE |
      BLECharacteristic::PROPERTY_READ  |
      BLECharacteristic::PROPERTY_NOTIFY
    );

  // Required by many clients to enable notifications
  pOtaCharacteristic->addDescriptor(new BLE2902());

  pService->start();

  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06); // helps with iPhone connections
  pAdvertising->setMinPreferred(0x12);

  BLEDevice::startAdvertising();
  pServer->getAdvertising()->start();

  pMovementCharacteristic->setCallbacks(
    new CharacteristicCallBack(movement, messageProcessor)
  ); 
  pSystemCharacteristic->setCallbacks(
    new CharacteristicCallBack(systemConfig, messageProcessor)
  );
  pConfigCharacteristic->setCallbacks(
    new CharacteristicCallBack(setting, messageProcessor)
  );
  pOtaCharacteristic->setCallbacks(
    new CharacteristicCallBack(ota, messageProcessor)
  );

  // Init TX queue and task AFTER characteristic exists
  gOtaCharForNotify = pOtaCharacteristic;

  if (!gBleTxQueue) {
    gBleTxQueue = xQueueCreate(8, sizeof(BleTxMsg));
    xTaskCreatePinnedToCore(bleTxTask, "ble_tx", 4096, nullptr, 1, nullptr, 1);
  }

  Serial.println("KinetiX now available for BT connection.");
}
