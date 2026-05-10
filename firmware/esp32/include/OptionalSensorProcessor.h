#pragma once

#include <Arduino.h>
#include "Hand.h"
#include "Settings.h"

#define SENSOR_PIN A0
#define MAX_READINGS 5 // Number of readings to average

class SensorProcessor {
public:
   virtual void run() = 0;
   virtual uint16_t getAvg() = 0;
   virtual ~SensorProcessor() = default;
};

class RealSensorProcessor : public SensorProcessor {
public:
   RealSensorProcessor(Hand *hand, Settings *settings, Display *display);
   void run() override;
   uint16_t getAvg() override;

   Hand *hand = NULL;
   time_t lastMeasureAt = 0;
   time_t measureIntervalMs = 70; 
   uint16_t readings[MAX_READINGS]; // Array to store readings
   uint16_t previousPosition = 0;   
   int index = 0; // Index for the current reading
   long sum = 0;
   int count = 0;
   Display *display = NULL;
   Settings *settings = NULL;
};


class MockSensorProcessor : public SensorProcessor {
public:
   MockSensorProcessor() {
      log_i("Mock Sensor Processor created");
   };
   void run() override {
      // Do nothing
   } 

   uint16_t getAvg() override {
      return 0;
   }
};