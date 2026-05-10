#pragma once
// Mock BLEServer.h for native unit testing
#include <string>

class BLECharacteristic {
public:
    void setValue(const char* val) { _value = val ? val : ""; }
    const std::string& getValue() const { return _value; }
private:
    std::string _value;
};

class BLEService {};
class BLEServer {};
class BLEAdvertising {};
