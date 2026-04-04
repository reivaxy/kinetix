#include "Hand.h"

Hand::Hand(Settings *settings) {
      this->settings = settings;
      fingers[0] = new Finger(0, THUMB_CTRL_PIN, 
                              settings->getPosition("thumb_max_open", THUMB_MAX_OPEN),
                              settings->getPosition("thumb_max_closed", THUMB_MAX_CLOSED), 1);
      fingers[1] = new Finger(1, FINGER1_CTRL_PIN,
                              settings->getPosition("finger1_max_open", FINGER1_MAX_OPEN),
                              settings->getPosition("finger1_max_closed", FINGER1_MAX_CLOSED), 1);
      fingers[2] = new Finger(2, FINGER2_CTRL_PIN,
                              settings->getPosition("finger2_max_open", FINGER2_MAX_OPEN),
                              settings->getPosition("finger2_max_closed", FINGER2_MAX_CLOSED), -1);
      fingers[3] = new Finger(3, FINGER3_CTRL_PIN,
                              settings->getPosition("finger3_max_open", FINGER3_MAX_OPEN),
                              settings->getPosition("finger3_max_closed", FINGER3_MAX_CLOSED), 1);
      fingers[4] = new Finger(4, FINGER4_CTRL_PIN,
                              settings->getPosition("finger4_max_open", FINGER4_MAX_OPEN),
                              settings->getPosition("finger4_max_closed", FINGER4_MAX_CLOSED), -1);
}

void Hand::setCalibration(bool servoCalibration) {
   if (servoCalibration) {
      // This setting should not be used once the wires have been installed, it could break stuff
      fingers[0]->setMax(SERVO_CALIBRATION_OPEN, SERVO_CALIBRATION_CLOSED);
      fingers[1]->setMax(SERVO_CALIBRATION_OPEN, SERVO_CALIBRATION_CLOSED);
      fingers[2]->setMax(SERVO_CALIBRATION_CLOSED, SERVO_CALIBRATION_OPEN);
      fingers[3]->setMax(SERVO_CALIBRATION_OPEN, SERVO_CALIBRATION_CLOSED);
      fingers[4]->setMax(SERVO_CALIBRATION_CLOSED, SERVO_CALIBRATION_OPEN);
   } else {
      fingers[0]->setMax(settings->getPosition("thumb_max_open", THUMB_MAX_OPEN),
                              settings->getPosition("thumb_max_closed", THUMB_MAX_CLOSED));
      fingers[1]->setMax(settings->getPosition("finger1_max_open", FINGER1_MAX_OPEN),
                              settings->getPosition("finger1_max_closed", FINGER1_MAX_CLOSED));
      fingers[2]->setMax(settings->getPosition("finger2_max_open", FINGER2_MAX_OPEN),
                              settings->getPosition("finger2_max_closed", FINGER2_MAX_CLOSED));
      fingers[3]->setMax(settings->getPosition("finger3_max_open", FINGER3_MAX_OPEN),
                              settings->getPosition("finger3_max_closed", FINGER3_MAX_CLOSED));
      fingers[4]->setMax(settings->getPosition("finger4_max_open", FINGER4_MAX_OPEN),
                              settings->getPosition("finger4_max_closed", FINGER4_MAX_CLOSED));
   }
}

void Hand::updateMaxPositionsFromSettings() {
   log_i("Updating finger max positions from settings");
   fingers[0]->setMax(settings->getPosition("thumb_max_open", THUMB_MAX_OPEN),
                      settings->getPosition("thumb_max_closed", THUMB_MAX_CLOSED));
   fingers[0]->refresh(); // refresh with new max positions                      
   fingers[1]->setMax(settings->getPosition("finger1_max_open", FINGER1_MAX_OPEN),
                      settings->getPosition("finger1_max_closed", FINGER1_MAX_CLOSED));
   fingers[1]->refresh(); // refresh with new max positions
   fingers[2]->setMax(settings->getPosition("finger2_max_open", FINGER2_MAX_OPEN),
                      settings->getPosition("finger2_max_closed", FINGER2_MAX_CLOSED));
   fingers[2]->refresh(); // refresh with new max positions
   fingers[3]->setMax(settings->getPosition("finger3_max_open", FINGER3_MAX_OPEN),
                      settings->getPosition("finger3_max_closed", FINGER3_MAX_CLOSED));
   fingers[3]->refresh(); // refresh with new max positions
   fingers[4]->setMax(settings->getPosition("finger4_max_open", FINGER4_MAX_OPEN),
                      settings->getPosition("finger4_max_closed", FINGER4_MAX_CLOSED));
   fingers[4]->refresh(); // refresh with new max positions
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

void Hand::moveRelative(int to) {
   log_i("Hand::moveRelative to %d", to);
   for (uint i=0 ; i < FINGER_COUNT; i++) {
      fingers[i]->computeTarget(to);
   }
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

void Hand::setStep(float step) {
   for (uint i=0 ; i < FINGER_COUNT; i++) {
      setStep(i, step);
   }
}
void Hand::setStep(int finger, float step) {
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
      log_d("Stopping finger %d", finger);
      fingers[finger]->stop();
   }
}