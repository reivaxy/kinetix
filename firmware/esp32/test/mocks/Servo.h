#pragma once
// Mock Servo.h for native unit testing – mimics the ESP32 Servo API

class Servo {
public:
    static constexpr int CHANNEL_NOT_ATTACHED       = -1;
    static constexpr int DEFAULT_MIN_PULSE_WIDTH_US = 544;
    static constexpr int DEFAULT_MAX_PULSE_WIDTH_US = 2400;

    // ESP32-style attach with full parameter list
    bool attach(int pin,
                int channel         = CHANNEL_NOT_ATTACHED,
                int minDeg          = 0,
                int maxDeg          = 180,
                int minPulseWidthUs = DEFAULT_MIN_PULSE_WIDTH_US,
                int maxPulseWidthUs = DEFAULT_MAX_PULSE_WIDTH_US,
                int frequency       = 50) { return true; }

    void detach() {}
    void write(int angle)     { _angle = angle; }
    int  read()         const { return _angle; }
    bool attached()     const { return true; }

private:
    int _angle = 0;
};
