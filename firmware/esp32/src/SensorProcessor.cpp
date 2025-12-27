
#include "SensorProcessor.h"


SensorProcessor::SensorProcessor(Hand *hand, Settings *settings, Display *display) {
   this->hand = hand;
   this->settings = settings;
   analogSetPinAttenuation(SENSOR_PIN, ADC_6db);
   this->display = display;
}

void SensorProcessor::run() {
   hand->run();
   uint16_t newReading = 0;
   // We must not read the ADC too fast
   if (millis() - lastMeasureAt > measureIntervalMs) {
      newReading = getAvg();
      lastMeasureAt = millis();
   } else {
      return;
   }

   // Let's read offset and threshold from all-purpose "experiment" settings
   int offset = settings->getInt("i_1", 380);
   int threshold = settings->getInt("i_2", 2);
   int position = offset - (newReading/10);
   if (position < 0) position = 0;
   
   // We don't want to change position for tiny variation.
   // ADC is typically 1% accurate to readings vary even if voltage does not   
   if (abs(position - previousPosition) > threshold) {
     previousPosition = position;
     // display position, offset and threshold for debugging
     display->setLine(SENSOR_DISPLAY_LINE, "Sensor: " + String(position) + " (" + String(offset) + ", " + String(threshold) + ")");
     log_i("Sensor position: %d", position);
     // high limit should be adjustable saved in config
     hand->moveRelative(map(position, 0, 100, 0, 180));
   }

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
