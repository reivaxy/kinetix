
#include "HandMovement.h"


HandMovement::HandMovement(Hand *_hand) {
   hand = _hand;
}

void HandMovement::start() {
   for(uint i = 0  ; i < 5 ; i++) {
     hand->setFinger(i, fingerMovement[i]);
   }
   startedAt = millis();
   running = true;
}

void HandMovement::run() {
   if (!running) {
      return;
   }
   for(uint i = 0  ; i < 5 ; i++) {
      if (millis() - startedAt > fingerMovement[i]->startDelay) {
         hand->run(i);
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