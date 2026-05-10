#pragma once
// Mock ArduinoJson.h for native unit testing
// Provides stub types sufficient to compile Settings.cpp on native.
#include <Arduino.h>
#include <cstring>

// ---------------------------------------------------------------------------
// JsonVariant – supports operator|, operator=, is<T>(), and cast to scalar
// ---------------------------------------------------------------------------
struct JsonVariant {
    // or-default: always return the default (data is never stored in stubs)
    template<typename T>
    T operator|(T defaultVal) const { return defaultVal; }

    // Specialisation for const char* to avoid ambiguity
    const char* operator|(const char* defaultVal) const { return defaultVal; }

    // Assignment operators (no-op – we don't store anything in tests)
    template<typename T>
    JsonVariant& operator=(T) { return *this; }

    // Type check – always returns false in stub
    template<typename T>
    bool is() const { return false; }

    // Cast operators – return zero/null/false for every type
    operator int()         const { return 0; }
    operator float()       const { return 0.0f; }
    operator bool()        const { return false; }
    operator const char*() const { return nullptr; }
    operator String()      const { return String(""); }
};

// ---------------------------------------------------------------------------
// JsonDocument – minimal stub
// ---------------------------------------------------------------------------
class JsonDocument {
public:
    JsonVariant operator[](const char* /*key*/) const { return JsonVariant{}; }
    JsonVariant operator[](const char* /*key*/)       { return JsonVariant{}; }
    void clear() {}
};

// ---------------------------------------------------------------------------
// DeserializationError – falsy on success (always success in stub)
// ---------------------------------------------------------------------------
struct DeserializationError {
    operator bool()        const { return false; }   // false = no error
    const char* c_str()    const { return ""; }
};

// ---------------------------------------------------------------------------
// Free functions
// ---------------------------------------------------------------------------
inline DeserializationError deserializeJson(JsonDocument& /*doc*/, const String& /*src*/) {
    return {};
}
inline DeserializationError deserializeJson(JsonDocument& /*doc*/, const char* /*src*/) {
    return {};
}
inline void serializeJson(const JsonDocument& /*doc*/, String& output) {
    output = String("{}");
}
inline void serializeJson(const JsonDocument& /*doc*/, char* output, size_t len) {
    if (output && len > 2) { strncpy(output, "{}", len); }
}
