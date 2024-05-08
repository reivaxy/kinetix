#include "Hand.h"


Hand::Hand() {
   fingers[0] = new Finger(THUMB_PIN, THUMB_MAX_OPEN, THUMB_MAX_CLOSED);
   fingers[1] = new Finger(FINGER2_PIN, FINGER2_MAX_OPEN, FINGER2_MAX_CLOSED);
   fingers[2] = new Finger(FINGER3_PIN, FINGER3_MAX_OPEN, FINGER3_MAX_CLOSED);
   fingers[3] = new Finger(FINGER4_PIN, FINGER4_MAX_OPEN, FINGER4_MAX_CLOSED);
   fingers[4] = new Finger(FINGER5_PIN, FINGER5_MAX_OPEN, FINGER5_MAX_CLOSED);

}

void Hand::close() {
   for (uint i=0 ; i < 5; i++) {
      close(i);
   }
}

void Hand::close(uint finger) {
   // Serial.println("Closing finger");
   fingers[finger]->close();
}

void Hand::open() {
   for (uint i=0 ; i < 5; i++) {
      open(i);
   }
}

void Hand::open(uint finger) {
   // Serial.println("Opening finger");
   fingers[finger]->open();
}

void Hand::run() {
   for (uint i=0 ; i < 5; i++) {
      fingers[i]->run();
   }
}

void Hand::run(uint finger) {
   fingers[finger]->run();
}

void Hand::setStep(float step) {
   for (uint i=0 ; i < 5; i++) {
      setStep(i, step);
   }
}
void Hand::setStep(int finger, float step) {
   fingers[finger]->setStep(step);
}

bool Hand::isStill() {
   bool result = true;
   for (uint i=0 ; i < 5; i++) {
      result = result && isStill(i);
   }   
   return result;
}

bool Hand::isStill(uint finger) {
   return fingers[finger]->isStill();
}

void Hand::setFinger(uint finger, FingerMovement *fingerMovement) {
   fingers[finger]->setMovement(fingerMovement);
}

void Hand::stop(uint finger) {
   fingers[finger]->stop();
}

void Hand::stop() {
   for (uint finger=0 ; finger < 5; finger++) {
      fingers[finger]->stop();
   }
}