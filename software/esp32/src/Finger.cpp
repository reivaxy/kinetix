
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
   monitorPin = _monitorPin;
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

void Finger::close() {
   move(maxClosed);
}

void Finger::setStep(int _step) {
   step = _step;
}
void Finger::run() {
   time_t r = millis() % 200;
   if (number == 4 && r == 0) return;
   bool update = false;
   char msg[100];
   // sprintf(msg, "Current pos %d, target %d\n", currentPosition, target);
   // Serial.print(msg);

   if (currentPosition < target) {
      currentPosition += step;
      update = true;
   }
   if (currentPosition > target) {
      currentPosition -= step;
      update = true;
   }


   if (update) {
      
      int current = analogRead(monitorPin);
      // if (number == 4) {
      //    sprintf(msg, "Current f%d: %d", number, current);
      //    Serial.println(msg);
      // }
      if (current > 600) {
         stop();
         sprintf(msg, "Stopping f%d, current %d", number, current);
         Serial.println(msg);      

      } else {
         myServo.write(currentPosition);    
      }
   
   }

}

bool Finger::isStill() {
   return currentPosition == target;
}

/**
 * @brief All fingers have a 1-255 course. Their actual servo positions were adjusted
 * with maxOpen and maxClosed at initialization.
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
   char msg[20];
   sprintf(msg, "Target f%d: %d", number, target);
   Serial.println(msg);
   step = fingerMovement->step;
}

void Finger::stop() {
   target = currentPosition;
}