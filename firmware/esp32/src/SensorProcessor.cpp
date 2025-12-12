
#include "SensorProcessor.h"


SensorProcessor::SensorProcessor(Hand *hand) {
   this->hand = hand;
   analogSetPinAttenuation(SENSOR_PIN, ADC_6db);
}

void SensorProcessor::run() {

   uint16_t newReading = previousReading;
   // We must not read the ADC too fast
   if (millis() - lastMeasureAt > measureIntervalMs) {
      newReading = getAvg();
      lastMeasureAt = millis();
      Serial.println(newReading);
   }

   // Serial.println(newPosition);
   // We don't want to change position for tiny variation.
   // ADC is typically 1% accurate to readings vary when voltage does not
   if (abs(newReading - previousReading > 10)) {
     previousReading = newReading;
     int position = 380 - (newReading/10);
     hand->moveRelative(map(position, 0, 100, 0, 180));
   }
   hand->run();
}

uint16_t SensorProcessor::getAvg() {
   uint16_t measure = analogRead(SENSOR_PIN);

   if (count == MAX_READINGS) {
      sum -= readings[index]; // Subtract oldest reading from total  
   }
   readings[index] = measure; // Store new reading
   sum += measure; // Add new reading to total
   index = (index + 1) % MAX_READINGS; // Increment index, loop if necessary
   if (count < MAX_READINGS) {
      count++;
   }
   return sum / count; // Return average.
}
