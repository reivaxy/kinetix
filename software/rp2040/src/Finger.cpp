
#include "Finger.h"


/**
 * @brief Construct a new Finger:: Finger object
 * 
 * @param pin 
 * // 6, 28 is 180°, but thumb pulley expects 120°
 * @param maxOpen: not below 6 
 * @param maxClosed: not above 28  (and 18 for thumb)
 */
Finger::Finger(int _pin, float _maxOpen, float _maxClosed) {
   pin = _pin;
   maxOpen = _maxOpen;
   maxClosed = _maxClosed;
   current = maxOpen; // At init

   PWMInstance = new RP2040_PWM(pin, frequency, maxOpen);
   if (PWMInstance) {
      PWMInstance->setPWM();
      PWMInstance->enablePWM();
   }
  Serial.print(F("\nStarting PWM_DynamicDutyCycle on "));
  Serial.println(BOARD_NAME);
  Serial.println(RP2040_PWM_VERSION);
}

void Finger::move(float to) {
   target = to;
}

void Finger::open() {
   move(maxOpen);
}

void Finger::close() {
   move(maxClosed);
}

void Finger::run() {
   bool update = false;
   if (current < target) {
      current += step;
      update = true;
      if (target - current < step) {
         current = target;
      }
   }
   if (current > target) {
      current -= step;
      update = true;
      if (current - target < step) {
         current = target;
      }      
   }
   // char msg[100];
   // sprintf(msg, "Duty Cycle: %f\n", current);
   // Serial.print(msg);
   if (update) {
      PWMInstance->setPWM(pin, frequency, current);
   }
}

void Finger::setStep(float _step) {
   step = _step;
}

bool Finger::isStill() {
   return current == target;
}

void Finger::setMovement(FingerMovement *fingerMovement) {
   target = maxOpen + (fingerMovement->relativeTargetPosition * (maxClosed- maxOpen) / 10) ;
   // Serial.print("Target: ");
   // Serial.println(target);
   step = fingerMovement->step;
}

void Finger::stop() {
   target = current;
}