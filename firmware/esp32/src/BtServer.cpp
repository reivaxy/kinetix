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

// Forward declaration
class BtServer;

// Global BtServer instance for callbacks
static BtServer* gBtServer = nullptr;

class CharacteristicCallBack : public BLECharacteristicCallbacks {
public:
  MessageProcessor* messageProcessor;
  BtServer* btServer;
  MessageType type;

  CharacteristicCallBack(MessageType type, MessageProcessor* _messageProcessor, BtServer* _btServer = nullptr) {
    this->messageProcessor = _messageProcessor;
    this->btServer = _btServer;
    this->type = type;
  }

  void onWrite(BLECharacteristic* characteristic) override {
    log_i("Kinetix received a write request for type %d", type);

    char message[MAX_MESSAGE_SIZE];
    size_t len = min((int)characteristic->getLength(), MAX_MESSAGE_SIZE - 1);
    strncpy(message, (char*)characteristic->getData(), len);
    message[len] = 0;

    // Always allow password characteristic to be written
    if (type == password) {
      log_i("Processing password write");
      if (messageProcessor != NULL) {
        messageProcessor->processWriteMsg(type, message);
      }
      return;
    }

    // Check authentication for all other characteristics
    if (btServer != nullptr && !btServer->isClientAuthenticated()) {
      log_w("Client not authenticated, rejecting write on characteristic type %d", type);
      return;
    }

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
    log_i("Kinetix received a read request for type %d", type);
    
     // Check authentication for all other characteristics than password which can always be read (to check auth state)
    if (type != password && btServer != nullptr && !btServer->isClientAuthenticated()) {
      log_w("Client not authenticated, rejecting read on characteristic type %d", type);
      return;
    }

    if (messageProcessor != NULL) {
      messageProcessor->processReadMsg(type, characteristic);
    }
  }
};

class MyServerCallback : public BLEServerCallbacks {
  Display *display;
  BtServer *btServer;
  
public:
  MyServerCallback(Display *display, BtServer *btServer) : display(display), btServer(btServer) {
    this->display = display;
    this->btServer = btServer;
  }

private:

  void onConnect(BLEServer* pServer) override {
    log_i("Client connected.");
    display->setLine(CONNECTED_DISPLAY_LINE, "BT Connected");
    if (btServer != nullptr) {
      btServer->pServer = pServer;
      // Get the connection ID from the connected client
      btServer->clientConnId = pServer->getConnId();
      btServer->resetAuthenticationState();
      btServer->setPasswordTimeout(PASSWORD_TIMEOUT_MS);
    }
  }
  
  void onDisconnect(BLEServer* pServer) override {
    log_i("Client disconnected");
    display->setLine(CONNECTED_DISPLAY_LINE, "BT Disconnected");
    if (btServer != nullptr) {
      btServer->resetAuthenticationState();
    }
    // Need to restart advertising to be able to reconnect
    pServer->getAdvertising()->start();
  }
};

BtServer::BtServer(MessageProcessor* _messageProcessor, Display *display) {
  messageProcessor = _messageProcessor;
  this->display = display;
  gBtServer = this;
  display->setLine(CONNECTED_DISPLAY_LINE, "BT Disconnected");
  BLEDevice::init("KinetiX");
  BLEServer* pServer = BLEDevice::createServer();
  this->pServer = pServer;  // Store server pointer for later use
  pServer->setCallbacks(new MyServerCallback(display, this));

  BLEService* pService = pServer->createService(SERVICE_UUID);

  pMovementCharacteristic =
    pService->createCharacteristic(
      MOVEMENT_CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_WRITE
    );

  pSystemCharacteristic =
    pService->createCharacteristic(
      SYSTEM_CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ
    );

  pConfigCharacteristic =
    pService->createCharacteristic(
      CONFIG_CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ
    );

  pPositionsCharacteristic =
    pService->createCharacteristic(
      POSITIONS_CHARACTERISTIC_UUID,
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

  // Password characteristic (WRITE only)
  BLECharacteristic* pPasswordCharacteristic =
    pService->createCharacteristic(
      PASSWORD_CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ
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
    new CharacteristicCallBack(movement, messageProcessor, this)
  ); 
  pSystemCharacteristic->setCallbacks(
    new CharacteristicCallBack(systemConfig, messageProcessor, this)
  );
  pConfigCharacteristic->setCallbacks(
    new CharacteristicCallBack(setting, messageProcessor, this)
  );
  pPositionsCharacteristic->setCallbacks(
    new CharacteristicCallBack(positions, messageProcessor, this)
  );
  pOtaCharacteristic->setCallbacks(
    new CharacteristicCallBack(ota, messageProcessor, this)
  );
  pPasswordCharacteristic->setCallbacks(
    new CharacteristicCallBack(password, messageProcessor, this)
  );

  // Init TX queue and task AFTER characteristic exists
  gOtaCharForNotify = pOtaCharacteristic;

  if (!gBleTxQueue) {
    gBleTxQueue = xQueueCreate(8, sizeof(BleTxMsg));
    xTaskCreatePinnedToCore(bleTxTask, "ble_tx", 4096, nullptr, 1, nullptr, 1);
  }

  Serial.println("KinetiX now available for BT connection.");
}

// -------------------- Authentication methods --------------------

void BtServer::setClientAuthenticated(bool authenticated) {
  clientAuthenticated = authenticated;
  if (authenticated) {
    log_i("Client authenticated");
    authenticationTimestamp = 0;  // Clear timeout
    enableAllCharacteristics();
  } else {
    log_i("Client not authenticated");
    authenticationTimestamp = millis();
    // Don't disable all characteristics here - let them stay accessible during auth window
  }
}

bool BtServer::isClientAuthenticated() {
  return clientAuthenticated;
}

void BtServer::setPasswordTimeout(uint32_t timeoutMs) {
  passwordTimeoutMs = timeoutMs;
  authenticationTimestamp = millis();
}

void BtServer::checkPasswordTimeout() {
  if (clientAuthenticated || authenticationTimestamp == 0) {
    return;  // Already authenticated or not waiting for password
  }
  
  uint32_t elapsed = millis() - authenticationTimestamp;
  if (elapsed > passwordTimeoutMs) {
    log_i("Password timeout exceeded, disconnecting client (conn_id: %d)", clientConnId);
    // Disconnect the specific client to allow others to connect
    if (pServer != nullptr) {
      pServer->disconnect(clientConnId);
    }
    authenticationTimestamp = 0;
  }
}

void BtServer::disableAllCharacteristics() {
  // Disable read/write on protected characteristics
  if (pMovementCharacteristic != nullptr) {
    pMovementCharacteristic->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
  }
  if (pSystemCharacteristic != nullptr) {
    pSystemCharacteristic->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
  }
  if (pConfigCharacteristic != nullptr) {
    pConfigCharacteristic->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
  }
  if (pPositionsCharacteristic != nullptr) {
    pPositionsCharacteristic->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
  }
}

void BtServer::enableAllCharacteristics() {
  // Enable read/write on protected characteristics
  if (pMovementCharacteristic != nullptr) {
    pMovementCharacteristic->setAccessPermissions(ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE);
  }
  if (pSystemCharacteristic != nullptr) {
    pSystemCharacteristic->setAccessPermissions(ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE);
  }
  if (pConfigCharacteristic != nullptr) {
    pConfigCharacteristic->setAccessPermissions(ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE);
  }
  if (pPositionsCharacteristic != nullptr) {
    pPositionsCharacteristic->setAccessPermissions(ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE);
  }
}

void BtServer::resetAuthenticationState() {
  clientAuthenticated = false;
  authenticationTimestamp = 0;
}
