

#include "Sequence.h"

Sequence::Sequence(uint8_t _repeatCount =1) {
   repeatCount = _repeatCount;
}

void Sequence::addMovement(HandMovement *movement, uint32_t pauseAfter = 100) {
   // Ignore when full
   if (movementCount == MAX_MOVEMENTS) {
      Serial.println("Reached max movement count, ignoring");
      return;
   }
   movements[movementCount] = movement;
   pauses[movementCount] = pauseAfter;
}

void Sequence::start(uint8_t start = 0) {
   running = true;
   current = start;
}

void Sequence::run() {
   if (!running) {
      return;
   }
   HandMovement *currentHm = movements[current];
   

}

void Sequence::stop() {
   running = false;
   movements[current]->stop();
}