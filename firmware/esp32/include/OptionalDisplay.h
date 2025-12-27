#pragma once

#include <Arduino.h>
#include "XOLEDDisplay.h"


class Display {
public:
    virtual void setTitle(String) = 0;
    virtual void setLine(int, String) = 0;
    virtual void refresh() = 0;
};

class MockDisplay : public Display {
public:
    MockDisplay() {
        log_i("Mock Display created");
    };
    void setTitle(String title) override {
      log_i("Mock Display initialized");
    };
    void setLine(int line, String content) override {
    };
    void refresh() {
    };
};

class RealDisplay : public Display {
private:
    XOLEDDisplayClass *realDisplay; 
    time_t lastRefresh = 0;
public:
    RealDisplay() : realDisplay(new XOLEDDisplayClass(0x3C, SDA, SCL, false, 120)) {
        log_i("Real Display created");
    };

    void setTitle(String title) override {
        log_i("Initializing real display");
        realDisplay->setTitle(title.c_str());
        log_i("Display initialized");
    };
    void setLine(int line, String content) override {
        realDisplay->setLine(line, content.c_str());
    };
    void refresh() override {
        time_t now = millis();
        // Don't refresh too often or sequences will struggle ("come", "scratch", etc)
        if (now - lastRefresh < 200) {
            return;
        }   
        lastRefresh = now;
        realDisplay->refresh();
    };
};
