#include "Hand.h"


Hand::Hand() {
   fingers[0] = new Finger(THUMB_PIN, THUMB_MAX_OPEN, THUMB_MAX_CLOSED);
   fingers[1] = new Finger(FINGER2_PIN, FINGER2_MAX_OPEN, FINGER2_MAX_CLOSED);
   fingers[2] = new Finger(FINGER3_PIN, FINGER3_MAX_OPEN, FINGER3_MAX_CLOSED);
   fingers[3] = new Finger(FINGER4_PIN, FINGER4_MAX_OPEN, FINGER4_MAX_CLOSED);
   fingers[4] = new Finger(FINGER5_PIN, FINGER5_MAX_OPEN, FINGER5_MAX_CLOSED);

}

void Hand::close() {
   for (int i=0 ; i < 5; i++) {
      close(i);
   }
}

void Hand::close(int finger) {
   // Serial.println("Closing finger");
   fingers[finger]->close();
}

void Hand::open() {
   for (int i=0 ; i < 5; i++) {
      open(i);
   }
}

void Hand::open(int finger) {
   // Serial.println("Opening finger");
   fingers[finger]->open();
}

void Hand::run() {
   for (int i=0 ; i < 5; i++) {
      fingers[i]->run();
   }
}

void Hand::setStep(float step) {
   for (int i=0 ; i < 5; i++) {
      setStep(i, step);
   }
}
void Hand::setStep(int finger, float step) {
   fingers[finger]->setStep(step);
}

bool Hand::isStill() {
   bool result = true;
   for (int i=0 ; i < 5; i++) {
      result = result && isStill(i);
   }   
   return result;
}

bool Hand::isStill(int finger) {
   return fingers[finger]->isStill();
}