#pragma once
// Mock Preferences.h for native unit testing
#include <Arduino.h>

class Preferences {
public:
    bool   begin(const char* /*name*/, bool /*readOnly*/ = false) { return true; }
    void   end() {}

    size_t putInt(const char* /*key*/, int32_t /*value*/)          { return 0; }
    int32_t getInt(const char* /*key*/, int32_t defaultValue = 0)  { return defaultValue; }

    size_t putString(const char* /*key*/, const char* /*value*/)   { return 0; }
    size_t putString(const char* /*key*/, const String& /*value*/) { return 0; }
    String getString(const char* /*key*/, const char* defaultValue = nullptr) {
        return String(defaultValue ? defaultValue : "");
    }
    String getString(const char* /*key*/, const String& defaultValue) {
        return defaultValue;
    }

    size_t putBool(const char* /*key*/, bool /*value*/) { return 0; }
    bool   getBool(const char* /*key*/, bool defaultValue = false) { return defaultValue; }

    bool   isKey(const char* /*key*/) { return false; }
};
