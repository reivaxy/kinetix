#include "Hand.h"


Hand::Hand() {
   fingers[0] = new Finger(THUMB_PIN, THUMB_MAX_OPEN, THUMB_MAX_CLOSED, 1);
   fingers[1] = new Finger(FINGER1_PIN, FINGER1_MAX_OPEN, FINGER1_MAX_CLOSED, 1);
   fingers[2] = new Finger(FINGER2_PIN, FINGER2_MAX_OPEN, FINGER2_MAX_CLOSED, -1);
   fingers[3] = new Finger(FINGER3_PIN, FINGER3_MAX_OPEN, FINGER3_MAX_CLOSED, 1);
   fingers[4] = new Finger(FINGER4_PIN, FINGER4_MAX_OPEN, FINGER4_MAX_CLOSED, -1);

}

void Hand::close() {
   for (uint i=0 ; i < FINGER_COUNT; i++) {
      close(i);
   }
}

void Hand::close(uint finger) {
   // Serial.println("Closing finger");
   fingers[finger]->close();
}

void Hand::open() {
   for (uint i=0 ; i < FINGER_COUNT; i++) {
      open(i);
   }
}

void Hand::open(uint finger) {
   // Serial.println("Opening finger");
   fingers[finger]->open();
}

void Hand::run() {
   for (uint i=0 ; i < FINGER_COUNT; i++) {
      fingers[i]->run();
   }
}

void Hand::run(uint finger) {
   fingers[finger]->run();
}

void Hand::setStep(int step) {
   for (uint i=0 ; i < FINGER_COUNT; i++) {
      setStep(i, step);
   }
}
void Hand::setStep(int finger, int step) {
   fingers[finger]->setStep(step);
}

bool Hand::isStill() {
   bool result = true;
   for (uint i=0 ; i < FINGER_COUNT; i++) {
      result = result && isStill(i);
   }   
   return result;
}

bool Hand::isStill(uint finger) {
   return fingers[finger]->isStill();
}

void Hand::setFinger(uint finger, FingerMovement *fingerMovement) {
   if(fingerMovement == NULL) return;
   fingers[finger]->setMovement(fingerMovement);
}

void Hand::stop(uint finger) {
   fingers[finger]->stop();
}

void Hand::stop() {
   for (uint finger=0 ; finger < FINGER_COUNT; finger++) {
      fingers[finger]->stop();
   }
}