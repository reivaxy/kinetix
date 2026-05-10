#pragma once
// Mock WebServer.h for native unit testing
#include <cstdint>

class WebServer {
public:
    explicit WebServer(uint16_t /*port*/ = 80) {}
    void begin() {}
    void stop() {}
    void handleClient() {}
    void on(const char* /*uri*/, void (*/*fn*/)()) {}
};
