
#include "Finger.h"


/**
 * @brief Construct a new Finger:: Finger object
 * 
 * @param pin 
 * @param maxOpen 
 * @param maxClosed
 */
Finger::Finger(int _number, int _controlPin, int _monitorPin, int _maxOpen, int _maxClosed, int _direction) {
   number = _number; // finger number, to help in logs
   controlPin = _controlPin;
   currentMonitor.setPin(_monitorPin);
   
   direction = _direction;
   maxOpen = _maxOpen;
   maxClosed = _maxClosed;
   if (direction == 1) {
      currentPosition = maxOpen; // At init
   } else {
      currentPosition = maxClosed;
   }
   myServo.attach(controlPin, Servo::CHANNEL_NOT_ATTACHED, 0,
               180, Servo::DEFAULT_MIN_PULSE_WIDTH_US,
               Servo::DEFAULT_MAX_PULSE_WIDTH_US, frequency);
}

void Finger::move(int to) {
   target = to;
}

void Finger::open() {
   move(maxOpen);
}

void Finger::setMaxOpen(int _maxOpen) {
   maxOpen = _maxOpen;
}

void Finger::setMaxClosed(int _maxClosed) {
   maxClosed = _maxClosed;
}

void Finger::setMax(int _maxOpen, int _maxClosed) {
   maxOpen = _maxOpen;
   maxClosed = _maxClosed;
}

void Finger::close() {
   move(maxClosed);
}

void Finger::setStep(float _step) {
   step = _step;
}
void Finger::run() {
   // time_t r = millis() % 10;
   // if (r != 0) return;
   bool update = false;
   char msg[100];

   if (currentPosition + step  <= target) {
      currentPosition += step;
      update = true;
   } else {
      if (currentPosition - step >= target) {
         currentPosition -= step;
         update = true;
      }
   }
   int current = currentMonitor.getValue();
   // if (number == 4 && current > 200) {
   //    sprintf(msg, "Stopping f%d, input: %d", number, current);
   //    Serial.println(msg);      
   //    stop();
   //    update = false;
   // } 

   if (update) {     
      myServo.write(currentPosition);      
   }

}

bool Finger::isStill() {
   return abs(currentPosition - target) <= step;
}

/**
 * @brief fingers have a 0-180° course, except thumb which is 0-120 (per pulley diameters).
 * Their actual servo positions were adjusted with maxOpen and maxClosed at initialization,
 * especially to account for servos being mounted oposite to each others (fingers 2 & 4) 
 * But their relative positions are between 0 (fully open) and 100 (fully closed).
 * @param fingerMovement 
 */
void Finger::setMovement(FingerMovement *fingerMovement) {
   if (fingerMovement == NULL) return;
   if (direction == 1) {
      target = map(fingerMovement->relativeTargetPosition, 0, 100, maxOpen, maxClosed);
   } else {
      target = map(fingerMovement->relativeTargetPosition, 0, 100, maxClosed, maxOpen);
      target = maxOpen - target + maxClosed;
   }
   log_d("Target f%d: %d", number, target);
   step = fingerMovement->step;
}

void Finger::stop() {
   target = currentPosition;
}