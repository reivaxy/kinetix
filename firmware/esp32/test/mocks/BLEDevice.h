#pragma once
// Mock BLEDevice.h for native unit testing
#include "BLEServer.h"

class BLEDevice {
public:
    static void init(const char* /*name*/) {}
    static BLEServer* createServer() { return nullptr; }
    static BLEAdvertising* getAdvertising() { return nullptr; }
    static void startAdvertising() {}
};
