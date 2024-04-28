
#include "HandMovement.h"


HandMovement::HandMovement(Hand *_hand, String _name) {
   hand = _hand;
   name = _name;
   for(uint i = 0  ; i < FINGER_COUNT ; i++) {
     fingerMovement[i] = NULL;
   }
}

HandMovement::~HandMovement() {
   for(uint i = 0  ; i < FINGER_COUNT ; i++) {
     delete(fingerMovement[i]);
   }
}

void HandMovement::setFM(uint offset, FingerMovement *fm) {
   fingerMovement[offset] = fm;
}

void HandMovement::start() {
   printMovement();   
   for(uint i = 0  ; i < FINGER_COUNT ; i++) {
     hand->setFinger(i, fingerMovement[i]);
   }
   startedAt = millis();
   running = true;
}

void HandMovement::run() {
   if (!running) {
      return;
   }
   for(uint i = 0  ; i < FINGER_COUNT ; i++) {
      if (fingerMovement[i] != NULL) {
         if (millis() - startedAt > fingerMovement[i]->startDelay) {
            hand->run(i);
         }  
      }
   }
}

void HandMovement::stop() {
   running = false;
   hand->stop();
}

// this doesn't work very well :(
boolean HandMovement::isFinished() {
   return hand->isStill();
}

void HandMovement::printMovement() {
   Serial.print("Move to ");
   Serial.println(name);
}