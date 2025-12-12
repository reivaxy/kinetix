#pragma once

#include <Arduino.h>
#include "Hand.h"

#define SENSOR_PIN A0
#define MAX_READINGS 5 // Number of readings to average

class SensorProcessor {
public:
   SensorProcessor(Hand *hand);
   void run();
   uint16_t getAvg();

   Hand *hand = NULL;
   time_t lastMeasureAt = 0;
   time_t measureIntervalMs = 70; 
   uint16_t readings[MAX_READINGS]; // Array to store readings
   uint16_t previousReading = 0;   
   int index = 0; // Index for the current reading
   long sum = 0;
   int count = 0;
};