

#pragma once

#include <Arduino.h>
#include <driver/adc.h>

#define NUM_READINGS 5 // Number of readings to average

class CurrentMonitor {
public:
   CurrentMonitor();
   void setPin(int pin);
   int getAvg();
   int getValue();


   int pin;
   uint16_t readings[NUM_READINGS]; // Array to store readings
   uint16_t previousReading = 0;
   int index = 0; // Index for the current reading
   long sum = 0;
   int count = 0;
   time_t lastMeasureAt = 0;
   time_t measureIntervalMs = 70; 
   
};