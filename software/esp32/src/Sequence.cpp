

#include "Sequence.h"

Sequence::Sequence(uint8_t _repeatCount) {
   repeatCount = _repeatCount;
   for (int m = 0 ; m < MAX_MOVEMENTS; m++) {
      movements[m] = NULL;
   }
}

Sequence::~Sequence() {
   log_i("Deleting sequence of %d movements", movementCount);
   stop();
   for(int m=0 ; m < movementCount; m++) {
      log_i("Deleting hand movement %d", m);
      movements[m]->stop();
      delete(movements[m]);
      movements[m] = NULL;
   }
   movementCount = 0;
}

void Sequence::addMovement(HandMovement *movement, uint32_t duration) {
   // Ignore when full
   if (movementCount == MAX_MOVEMENTS) {
      log_e("Error: Reached max movement count, ignoring");
      return;
   }
   movements[movementCount] = movement;
   durations[movementCount] = duration;
   movementCount ++;
}

void Sequence::start(uint8_t start) {
   log_i("Starting sequence of %d movements", movementCount);
   current = start;
   if (movements[current] != NULL) {
      running = true;
      movements[current]->start();
      previousMovementStarteddAt = millis();
   } else {
      log_e("Error: empty sequence started.");
   }
}

void Sequence::run() {
   if (!running) {
      return;
   }
   HandMovement *currentMovement = movements[current];
   if (currentMovement == NULL) {
      return;
   }
   uint32_t sinceStarted = millis() - previousMovementStarteddAt;
   if (sinceStarted > durations[current]) {
      currentMovement->stop();
      current ++;
      if (current >= movementCount) {
         current = 0;
         loopCount++;
      }
      if (repeatCount && loopCount >= repeatCount) {
         log_i("Sequence finished\n");
         stop();
         return;
      }
      previousMovementStarteddAt = millis();
      movements[current]->start();
   } 
   currentMovement->run();
   
}

void Sequence::stop() {
   HandMovement *currentMovement = movements[current];
   if (currentMovement != NULL) {
      currentMovement->stop();
   }
   running = false;
}