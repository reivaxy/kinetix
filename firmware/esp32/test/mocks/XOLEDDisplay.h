#pragma once
// Mock XOLEDDisplay.h for native unit testing

class XOLEDDisplayClass {
public:
    XOLEDDisplayClass(int /*addr*/, int /*sda*/, int /*scl*/,
                      bool /*flip*/ = false, int /*timeout*/ = 120) {}
    void setTitle(const char* /*title*/) {}
    void setLine(int /*line*/, const char* /*content*/) {}
    void refresh() {}
};
