
#include "Finger.h"


/**
 * @brief Construct a new Finger:: Finger object
 * 
 * @param pin 
 * @param maxOpen 
 * @param maxClosed
 */
Finger::Finger(int _pin, int _maxOpen, int _maxClosed, int _direction) {
   pin = _pin;
   direction = _direction;
   maxOpen = _maxOpen;
   maxClosed = _maxClosed;
   if (direction == 1) {
      current = maxOpen; // At init
   } else {
      current = maxClosed;
   }
   myServo.attach(pin, Servo::CHANNEL_NOT_ATTACHED, 0,
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
   bool update = false;
   char msg[100];
   // sprintf(msg, "Current %d, target %d\n", current, target);
   // Serial.print(msg);

   if (current < target) {
      current += step;
      update = true;
   }
   if (current > target) {
      current -= step;
      update = true;
   }

   if (update) {
      // sprintf(msg, "new %d\n", current);
      // Serial.print(msg);
      myServo.write(current);
   }
}

bool Finger::isStill() {
   return current == target;
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
   Serial.print("Target: ");
   Serial.println(target);
   step = fingerMovement->step;
}

void Finger::stop() {
   target = current;
}