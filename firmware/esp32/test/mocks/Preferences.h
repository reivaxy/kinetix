#pragma once
// Mock Preferences.h for native unit testing
#include <Arduino.h>
#include <map>
#include <string>

class Preferences {
public:
    bool begin(const char* /*name*/, bool /*readOnly*/ = false) { return true; }
    void end() {}

    size_t putInt(const char* key, int32_t value) {
        store[key] = std::to_string(value); return 0;
    }
    int32_t getInt(const char* key, int32_t defaultValue = 0) {
        auto it = store.find(key);
        if (it != store.end()) return std::stoi(it->second);
        return defaultValue;
    }

    size_t putString(const char* key, const char* value) {
        store[key] = value ? value : ""; return 0;
    }
    size_t putString(const char* key, const String& value) {
        store[key] = value.c_str(); return 0;
    }
    String getString(const char* key, const char* defaultValue = nullptr) {
        auto it = store.find(key);
        if (it != store.end()) return String(it->second.c_str());
        return String(defaultValue ? defaultValue : "");
    }
    String getString(const char* key, const String& defaultValue) {
        auto it = store.find(key);
        if (it != store.end()) return String(it->second.c_str());
        return defaultValue;
    }

    size_t putBool(const char* key, bool value) {
        store[key] = value ? "1" : "0"; return 0;
    }
    bool getBool(const char* key, bool defaultValue = false) {
        auto it = store.find(key);
        if (it != store.end()) return it->second != "0";
        return defaultValue;
    }

    bool isKey(const char* key) { return store.find(key) != store.end(); }

private:
    std::map<std::string, std::string> store;
};
