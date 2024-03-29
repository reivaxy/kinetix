
#include "HandMovementFactory.h"

HandMovementFactory::HandMovementFactory(Hand *_hand) {
   hand = _hand;
}

HandMovement* HandMovementFactory::fist() {
   HandMovement *handMovement = new HandMovement(hand, "Fist");
   // Thumb
   handMovement->fingerMovement[0] = new FingerMovement(10, 300); // close with little delay to be above finger 1
   handMovement->fingerMovement[1] = new FingerMovement(10, 0);
   handMovement->fingerMovement[2] = new FingerMovement(10, 0);
   handMovement->fingerMovement[3] = new FingerMovement(10, 0);
   handMovement->fingerMovement[4] = new FingerMovement(10, 0);
   return handMovement;
}

HandMovement* HandMovementFactory::ok() {
   HandMovement *handMovement = new HandMovement(hand, "Ok");
   // Thumb
   handMovement->fingerMovement[0] = new FingerMovement(0, 0); // Only thumb is open
   handMovement->fingerMovement[1] = new FingerMovement(10, 0);
   handMovement->fingerMovement[2] = new FingerMovement(10, 0);
   handMovement->fingerMovement[3] = new FingerMovement(10, 0);
   handMovement->fingerMovement[4] = new FingerMovement(10, 0);
   return handMovement;
}

HandMovement* HandMovementFactory::one(String name) {
   HandMovement *handMovement = new HandMovement(hand, name);
   // Thumb
   handMovement->fingerMovement[0] = new FingerMovement(10, 300); // close with little delay to be above finger 1
   handMovement->fingerMovement[1] = new FingerMovement(0, 0);  // Only finger 1 is open
   handMovement->fingerMovement[2] = new FingerMovement(10, 0);
   handMovement->fingerMovement[3] = new FingerMovement(10, 0);
   handMovement->fingerMovement[4] = new FingerMovement(10, 0);
   return handMovement;
}

HandMovement* HandMovementFactory::two() {
   HandMovement *handMovement = new HandMovement(hand, "Two");
   // Thumb
   handMovement->fingerMovement[0] = new FingerMovement(10, 300);
   handMovement->fingerMovement[1] = new FingerMovement(0, 0);
   handMovement->fingerMovement[2] = new FingerMovement(0, 0);
   handMovement->fingerMovement[3] = new FingerMovement(10, 0);
   handMovement->fingerMovement[4] = new FingerMovement(10, 0);
   return handMovement;
}

HandMovement* HandMovementFactory::three() {
   HandMovement *handMovement = new HandMovement(hand, "Three");
   // Thumb
   handMovement->fingerMovement[0] = new FingerMovement(10, 300);
   handMovement->fingerMovement[1] = new FingerMovement(0, 0);
   handMovement->fingerMovement[2] = new FingerMovement(0, 0);
   handMovement->fingerMovement[3] = new FingerMovement(0, 0);
   handMovement->fingerMovement[4] = new FingerMovement(10, 0);
   return handMovement;
}

HandMovement* HandMovementFactory::four() {
   HandMovement *handMovement = new HandMovement(hand, "Four");
   // Thumb
   handMovement->fingerMovement[0] = new FingerMovement(10, 300);
   handMovement->fingerMovement[1] = new FingerMovement(0, 0);
   handMovement->fingerMovement[2] = new FingerMovement(0, 0);
   handMovement->fingerMovement[3] = new FingerMovement(0, 0);
   handMovement->fingerMovement[4] = new FingerMovement(0, 0);
   return handMovement;
}

HandMovement* HandMovementFactory::five() {
   HandMovement *handMovement = new HandMovement(hand, "Five");
   // Thumb
   handMovement->fingerMovement[0] = new FingerMovement(0, 0);
   handMovement->fingerMovement[1] = new FingerMovement(0, 0);
   handMovement->fingerMovement[2] = new FingerMovement(0, 0);
   handMovement->fingerMovement[3] = new FingerMovement(0, 0);
   handMovement->fingerMovement[4] = new FingerMovement(0, 0);
   return handMovement;

}

HandMovement* HandMovementFactory::closePinch() {
   HandMovement *handMovement = new HandMovement(hand, "Closed pinch");
   // Thumb
   handMovement->fingerMovement[0] = new FingerMovement(9.2, 0);
   handMovement->fingerMovement[1] = new FingerMovement(7, 0);
   handMovement->fingerMovement[2] = new FingerMovement(10, 0);
   handMovement->fingerMovement[3] = new FingerMovement(10, 0);
   handMovement->fingerMovement[4] = new FingerMovement(10, 0);
   return handMovement;
}

HandMovement* HandMovementFactory::openPinch() {
   HandMovement *handMovement = new HandMovement(hand, "Open pinch");
   // Thumb
   handMovement->fingerMovement[0] = new FingerMovement(9.2, 0);
   handMovement->fingerMovement[1] = new FingerMovement(7, 0);
   handMovement->fingerMovement[2] = new FingerMovement(0, 0);
   handMovement->fingerMovement[3] = new FingerMovement(0, 0);
   handMovement->fingerMovement[4] = new FingerMovement(0, 0);
   return handMovement;
}

HandMovement* HandMovementFactory::idle() {
   HandMovement *handMovement = new HandMovement(hand, "Idle");
   // Thumb
   handMovement->fingerMovement[0] = new FingerMovement(1, 0);
   handMovement->fingerMovement[1] = new FingerMovement(1, 0);
   handMovement->fingerMovement[2] = new FingerMovement(2, 0);
   handMovement->fingerMovement[3] = new FingerMovement(1, 0);
   handMovement->fingerMovement[4] = new FingerMovement(1, 0);
   return handMovement;
}

HandMovement* HandMovementFactory::fu() {
   HandMovement *handMovement = new HandMovement(hand, "FU");
   // Thumb
   handMovement->fingerMovement[0] = new FingerMovement(10, 300);
   handMovement->fingerMovement[1] = new FingerMovement(10, 0);
   handMovement->fingerMovement[2] = new FingerMovement(0, 0);
   handMovement->fingerMovement[3] = new FingerMovement(10, 0);
   handMovement->fingerMovement[4] = new FingerMovement(10, 0);
   return handMovement;
}

HandMovement* HandMovementFactory::pointing() {
   return one("Pointing");
}