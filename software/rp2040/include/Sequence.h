
#include "HandMovement.h"
#define MAX_MOVEMENTS 50

class Sequence {
public:
   Sequence(uint8_t _repeatCount);
   void addMovement(HandMovement* movement, uint32_t pauseAfter);
   void start(uint8_t start);
   void stop();
   void run();

   HandMovement *movements[MAX_MOVEMENTS];
   uint32_t pauses[MAX_MOVEMENTS];
   uint32_t previousMovementStarteddAt = 0;

   uint8_t repeatCount = 1; // 0 means forever
   boolean running = false;
   uint8_t current = 0;
   uint8_t movementCount = 0;


};
