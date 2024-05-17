
#include "HandMovementFactory.h"

HandMovementFactory::HandMovementFactory(Hand *_hand) {
   hand = _hand;
}

HandMovement* HandMovementFactory::fist() {
   HandMovement *handMovement = new HandMovement(hand, "Fist");
   // Thumb
   handMovement->setFM(0, new FingerMovement(100, 300)); // close with little delay to be above finger 1
   handMovement->setFM(1, new FingerMovement(100, 0));
   handMovement->setFM(2, new FingerMovement(100, 0));
   handMovement->setFM(3, new FingerMovement(100, 0));
   handMovement->setFM(4, new FingerMovement(100, 0));
   return handMovement;
}

HandMovement* HandMovementFactory::ok() {
   HandMovement *handMovement = new HandMovement(hand, "Ok");
   // Thumb
   handMovement->setFM(0, new FingerMovement(0, 0)); // Only thumb is open
   handMovement->setFM(1, new FingerMovement(100, 0));
   handMovement->setFM(2, new FingerMovement(100, 0));
   handMovement->setFM(3, new FingerMovement(100, 0));
   handMovement->setFM(4, new FingerMovement(100, 0));
   return handMovement;
}

HandMovement* HandMovementFactory::one(String name) {
   HandMovement *handMovement = new HandMovement(hand, name);
   // Thumb
   handMovement->setFM(0, new FingerMovement(100, 200)); // close with little delay to be above finger 1
   handMovement->setFM(1, new FingerMovement(0, 0));  // Only finger 1 is open
   handMovement->setFM(2, new FingerMovement(100, 0));
   handMovement->setFM(3, new FingerMovement(100, 0));
   handMovement->setFM(4, new FingerMovement(100, 0));
   return handMovement;
}

HandMovement* HandMovementFactory::two() {
   HandMovement *handMovement = new HandMovement(hand, "Two");
   // Thumb
   handMovement->setFM(0, new FingerMovement(100, 200));
   handMovement->setFM(1, new FingerMovement(0, 0));
   handMovement->setFM(2, new FingerMovement(0, 0));
   handMovement->setFM(3, new FingerMovement(100, 0));
   handMovement->setFM(4, new FingerMovement(100, 0));
   return handMovement;
}

HandMovement* HandMovementFactory::three() {
   HandMovement *handMovement = new HandMovement(hand, "Three");
   // Thumb
   handMovement->setFM(0, new FingerMovement(100, 200));
   handMovement->setFM(1, new FingerMovement(0, 0));
   handMovement->setFM(2, new FingerMovement(0, 0));
   handMovement->setFM(3, new FingerMovement(0, 0));
   handMovement->setFM(4, new FingerMovement(100, 0));
   return handMovement;
}

HandMovement* HandMovementFactory::four() {
   HandMovement *handMovement = new HandMovement(hand, "Four");
   // Thumb
   handMovement->setFM(0, new FingerMovement(100, 200));
   handMovement->setFM(1, new FingerMovement(0, 0));
   handMovement->setFM(2, new FingerMovement(0, 0));
   handMovement->setFM(3, new FingerMovement(0, 0));
   handMovement->setFM(4, new FingerMovement(0, 0));
   return handMovement;
}

HandMovement* HandMovementFactory::five() {
   HandMovement *handMovement = new HandMovement(hand, "Five");
   // Thumb
   handMovement->setFM(0, new FingerMovement(0, 0));
   handMovement->setFM(1, new FingerMovement(0, 200));
   handMovement->setFM(2, new FingerMovement(0, 200));
   handMovement->setFM(3, new FingerMovement(0, 200));
   handMovement->setFM(4, new FingerMovement(0, 200));
   return handMovement;

}

HandMovement* HandMovementFactory::closePinch() {
   HandMovement *handMovement = new HandMovement(hand, "Closed pinch");
   // Thumb
   handMovement->setFM(0, new FingerMovement(92, 0));
   handMovement->setFM(1, new FingerMovement(70, 0));
   handMovement->setFM(2, new FingerMovement(100, 0));
   handMovement->setFM(3, new FingerMovement(100, 0));
   handMovement->setFM(4, new FingerMovement(100, 0));
   return handMovement;
}

HandMovement* HandMovementFactory::openPinch() {
   HandMovement *handMovement = new HandMovement(hand, "Open pinch");
   // Thumb
   handMovement->setFM(0, new FingerMovement(90, 0));
   handMovement->setFM(1, new FingerMovement(80, 0));
   handMovement->setFM(2, new FingerMovement(0, 0));
   handMovement->setFM(3, new FingerMovement(0, 0));
   handMovement->setFM(4, new FingerMovement(0, 0));
   return handMovement;
}

HandMovement* HandMovementFactory::idle() {
   HandMovement *handMovement = new HandMovement(hand, "Idle");
   // Thumb
   handMovement->setFM(0, new FingerMovement(10, 0));
   handMovement->setFM(1, new FingerMovement(10, 0));
   handMovement->setFM(2, new FingerMovement(0, 0));
   handMovement->setFM(3, new FingerMovement(10, 0));
   handMovement->setFM(4, new FingerMovement(10, 0));
   return handMovement;
}

HandMovement* HandMovementFactory::fu() {
   HandMovement *handMovement = new HandMovement(hand, "FU");
   // Thumb
   handMovement->setFM(0, new FingerMovement(100, 200));
   handMovement->setFM(1, new FingerMovement(100, 0));
   handMovement->setFM(2, new FingerMovement(0, 0));
   handMovement->setFM(3, new FingerMovement(100, 0));
   handMovement->setFM(4, new FingerMovement(100, 0));
   return handMovement;
}

HandMovement* HandMovementFactory::half() {
   HandMovement *handMovement = new HandMovement(hand, "Half");
   // Thumb
   handMovement->setFM(0, new FingerMovement(50, 0));
   handMovement->setFM(1, new FingerMovement(50, 0));
   handMovement->setFM(2, new FingerMovement(50, 0));
   handMovement->setFM(3, new FingerMovement(50, 0));
   handMovement->setFM(4, new FingerMovement(50, 0));
   return handMovement;
}

HandMovement* HandMovementFactory::rock() {
   HandMovement *handMovement = new HandMovement(hand, "Rock");
   // Thumb
   handMovement->setFM(0, new FingerMovement(100, 0));
   handMovement->setFM(1, new FingerMovement(0, 0));
   handMovement->setFM(2, new FingerMovement(100, 0));
   handMovement->setFM(3, new FingerMovement(100, 0));
   handMovement->setFM(4, new FingerMovement(0, 0));
   return handMovement;
}

HandMovement* HandMovementFactory::love() {
   HandMovement *handMovement = new HandMovement(hand, "Love");
   // Thumb
   handMovement->setFM(0, new FingerMovement(0, 0));
   handMovement->setFM(1, new FingerMovement(0, 0));
   handMovement->setFM(2, new FingerMovement(100, 0));
   handMovement->setFM(3, new FingerMovement(100, 0));
   handMovement->setFM(4, new FingerMovement(0, 0));
   return handMovement;
}

HandMovement* HandMovementFactory::scratchClose() {
   HandMovement *handMovement = new HandMovement(hand, "Scratch Close");
   // Thumb
   handMovement->setFM(0, new FingerMovement(80, 0, 0.006));
   handMovement->setFM(1, new FingerMovement(80, 0, 0.006));
   handMovement->setFM(2, new FingerMovement(75, 0, 0.006));
   handMovement->setFM(3, new FingerMovement(80, 0, 0.006));
   handMovement->setFM(4, new FingerMovement(80, 0, 0.006));
   return handMovement;
}

HandMovement* HandMovementFactory::scratchOpen() {
   HandMovement *handMovement = new HandMovement(hand, "Scratch Open");
   // Thumb
   handMovement->setFM(0, new FingerMovement(25, 0, 0.006));
   handMovement->setFM(1, new FingerMovement(25, 0, 0.006));
   handMovement->setFM(2, new FingerMovement(25, 0, 0.006));
   handMovement->setFM(3, new FingerMovement(25, 0, 0.006));
   handMovement->setFM(4, new FingerMovement(25, 0, 0.006));
   return handMovement;
}
HandMovement* HandMovementFactory::comeClose() {
   HandMovement *handMovement = new HandMovement(hand, "Come Close");
   // Thumb
   handMovement->setFM(0, new FingerMovement(80, 0));
   handMovement->setFM(1, new FingerMovement(100, 0, 0.01));
   handMovement->setFM(2, new FingerMovement(100, 0));
   handMovement->setFM(3, new FingerMovement(100, 0));
   handMovement->setFM(4, new FingerMovement(100, 0));
   return handMovement;
}

HandMovement* HandMovementFactory::comeOpen() {
   HandMovement *handMovement = new HandMovement(hand, "Come Open");
   // Thumb
   handMovement->setFM(0, new FingerMovement(80, 0));
   handMovement->setFM(1, new FingerMovement(0, 0, 0.01));
   handMovement->setFM(2, new FingerMovement(100, 0));
   handMovement->setFM(3, new FingerMovement(100, 0));
   handMovement->setFM(4, new FingerMovement(100, 0));
   return handMovement;
}

// Example of having two identical movements with different name
HandMovement* HandMovementFactory::pointing() {
   return one("Pointing");
}

// This movement only modifies the position of one finger by some quantity
HandMovement* HandMovementFactory::closeFingerBy(int finger, int closedQuantity) {
   String name = "closeFinger " + String(finger) + " by " + String(closedQuantity);
   HandMovement *handMovement = new HandMovement(hand, name);
   handMovement->setFM(finger, new FingerMovement(closedQuantity, 0));
   return handMovement;
}

HandMovement* HandMovementFactory::getByName(char *movementName) {
   if (0 == strcmp(movementName, "fist")) {
      return fist();
   }
   if (0 == strcmp(movementName, "ok")) {
      return ok();
   }
   if (0 == strcmp(movementName, "one")) {
      return one();
   }
   if (0 == strcmp(movementName, "two")) {
      return two();
   }
   if (0 == strcmp(movementName, "three")) {
      return three();
   }
   if (0 == strcmp(movementName, "four")) {
      return four();
   }
   if (0 == strcmp(movementName, "five")) {
      return five();
   }
   if (0 == strcmp(movementName, "closePinch")) {
      return closePinch();
   }
   if (0 == strcmp(movementName, "openPinch")) {
      return openPinch();
   }
   if (0 == strcmp(movementName, "idle")) {
      return idle();
   }
   if (0 == strcmp(movementName, "pointing")) {
      return pointing();
   }
   if (0 == strcmp(movementName, "fu")) {
      return fu();
   }
   if (0 == strcmp(movementName, "rock")) {
      return rock();
   }
   if (0 == strcmp(movementName, "love")) {
      return love();
   }

   // Requested movement was not found
   log_e("Movement not found: %s", movementName);
   return NULL;
}
