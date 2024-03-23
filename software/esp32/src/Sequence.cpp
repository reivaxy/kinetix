

#include "Sequence.h"

Sequence::Sequence(uint8_t _repeatCount) {
   repeatCount = _repeatCount;
}

void Sequence::addMovement(HandMovement *movement, uint32_t duration) {
   // Ignore when full
   if (movementCount == MAX_MOVEMENTS) {
      Serial.println("Reached max movement count, ignoring");
      return;
   }
   movements[movementCount] = movement;
   durations[movementCount] = duration;
   movementCount ++;
}

void Sequence::start(uint8_t start) {
   running = true;
   current = start;
   movements[current]->start();
   previousMovementStarteddAt = millis();
}

void Sequence::run() {
   if (!running) {
      return;
   }
   HandMovement *currentMovement = movements[current];
   uint32_t sinceStarted = millis() - previousMovementStarteddAt;
   if (sinceStarted > durations[current]) {
      currentMovement->stop();
      current ++;
      if (current >= movementCount) {
         current = 0;
         loopCount++;
      }
      if (repeatCount && loopCount >= repeatCount) {
         Serial.println("Sequence finished.");
         running = false;
         return;
      }
      previousMovementStarteddAt = millis();
      movements[current]->start();
   } 
   currentMovement->run();
   
}

void Sequence::stop() {
   running = false;
   movements[current]->stop();
}