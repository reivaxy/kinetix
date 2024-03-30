
#include "CurrentMonitor.h"

CurrentMonitor::CurrentMonitor() {   
}

void CurrentMonitor::setPin(int _pin) {
   pin = _pin;
}

int CurrentMonitor::getAvg() {
   uint16_t current = getValue();
   char msg[50];
   // sprintf(msg, "Reading: %d, index %d, count %d, sum %d", current, index, count, sum);
   // Serial.println(msg);

   if (count == NUM_READINGS) {
      sum -= readings[index]; // Subtract oldest reading from total  
   }
   readings[index] = current; // Store new reading
   sum += current; // Add new reading to total
   index = (index + 1) % NUM_READINGS; // Increment index, loop if necessary
   if (count < NUM_READINGS) {
      count++;
   }
   return sum / count; // Return average.
}

int CurrentMonitor::getValue() {
   if (millis() - lastMeasureAt < measureIntervalMs) {
      return previousReading;
   }

   previousReading = analogRead(pin);
   lastMeasureAt = millis();
   return previousReading;   
}