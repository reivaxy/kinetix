#pragma once
// Mock Arduino.h for native unit testing

#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <string>

// Basic Arduino types
typedef bool boolean;
typedef uint8_t byte;
typedef long time_t;
typedef unsigned int uint;

// Pin stubs
#define D6  6
#define D7  7
#define D8  8
#define D9  9
#define D10 10
#define A0  0
#define SDA 8
#define SCL 9

// Log macros – no-ops on native
#define log_i(format, ...)
#define log_e(format, ...)
#define log_w(format, ...)
#define log_d(format, ...)

// Controllable clock for tests
extern uint32_t mock_millis_value;
inline uint32_t millis()          { return mock_millis_value; }

// Analog / digital I/O stubs
inline int     analogRead(int /*pin*/)             { return 0; }
inline void    analogWrite(int /*pin*/, int /*v*/)  {}
inline int     digitalRead(int /*pin*/)            { return 0; }
inline void    digitalWrite(int /*pin*/, int /*v*/){}
inline void    pinMode(int /*pin*/, int /*mode*/)  {}

// Arduino math helper
inline long map(long x, long in_min, long in_max, long out_min, long out_max) {
    if (in_max == in_min) return out_min;
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// -------------------------------------------------------
// Minimal Arduino String compatible with the codebase
// -------------------------------------------------------
class String {
public:
    String() {}
    String(const char* s)         : _data(s ? s : "") {}
    String(const std::string& s)  : _data(s) {}
    String(int n)                 : _data(std::to_string(n)) {}
    String(float n)               : _data(std::to_string(n)) {}

    const char* c_str() const     { return _data.c_str(); }
    size_t      length()  const   { return _data.size(); }
    bool        isEmpty() const   { return _data.empty(); }

    String operator+(const String& rhs) const { return String(_data + rhs._data); }
    String& operator+=(const String& rhs)     { _data += rhs._data; return *this; }

    bool operator==(const String& rhs) const  { return _data == rhs._data; }
    bool operator!=(const String& rhs) const  { return _data != rhs._data; }

    // Allow implicit comparison with char*
    bool operator==(const char* rhs) const    { return _data == (rhs ? rhs : ""); }
    bool operator!=(const char* rhs) const    { return _data != (rhs ? rhs : ""); }

    // Conversion back to std::string for interop
    operator std::string() const { return _data; }

private:
    std::string _data;
};

// Allow: "literal" + String
inline String operator+(const char* lhs, const String& rhs) {
    return String(std::string(lhs ? lhs : "") + std::string(rhs));
}
