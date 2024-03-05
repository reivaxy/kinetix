
#include "HandMovementFactory.h"

HandMovementFactory::HandMovementFactory(Hand *_hand) {
   hand = _hand;
}

HandMovement* HandMovementFactory::fist() {
   Serial.println("Move to Fist");
   HandMovement *handMovement = new HandMovement(hand);
   // Thumb
   handMovement->fingerMovement[0] = new FingerMovement(10, 300); // close with little delay to be above finger 1
   handMovement->fingerMovement[1] = new FingerMovement(10, 0);
   handMovement->fingerMovement[2] = new FingerMovement(10, 0);
   handMovement->fingerMovement[3] = new FingerMovement(10, 0);
   handMovement->fingerMovement[4] = new FingerMovement(10, 0);
   return handMovement;
}

HandMovement* HandMovementFactory::ok() {
   Serial.println("Move to Ok");
   HandMovement *handMovement = new HandMovement(hand);
   // Thumb
   handMovement->fingerMovement[0] = new FingerMovement(0, 0); // Only thumb is open
   handMovement->fingerMovement[1] = new FingerMovement(10, 0);
   handMovement->fingerMovement[2] = new FingerMovement(10, 0);
   handMovement->fingerMovement[3] = new FingerMovement(10, 0);
   handMovement->fingerMovement[4] = new FingerMovement(10, 0);
   return handMovement;
}

HandMovement* HandMovementFactory::one(boolean silent) {
   if (!silent) {
      Serial.println("Move to One");
   }
   HandMovement *handMovement = new HandMovement(hand);
   // Thumb
   handMovement->fingerMovement[0] = new FingerMovement(10, 300); // close with little delay to be above finger 1
   handMovement->fingerMovement[1] = new FingerMovement(0, 0);  // Only finger 1 is open
   handMovement->fingerMovement[2] = new FingerMovement(10, 0);
   handMovement->fingerMovement[3] = new FingerMovement(10, 0);
   handMovement->fingerMovement[4] = new FingerMovement(10, 0);
   return handMovement;
}

HandMovement* HandMovementFactory::two() {
   Serial.println("Move to Two");
   HandMovement *handMovement = new HandMovement(hand);
   // Thumb
   handMovement->fingerMovement[0] = new FingerMovement(10, 300);
   handMovement->fingerMovement[1] = new FingerMovement(0, 0);
   handMovement->fingerMovement[2] = new FingerMovement(0, 0);
   handMovement->fingerMovement[3] = new FingerMovement(10, 0);
   handMovement->fingerMovement[4] = new FingerMovement(10, 0);
   return handMovement;
}

HandMovement* HandMovementFactory::three() {
   Serial.println("Move to Three");
   HandMovement *handMovement = new HandMovement(hand);
   // Thumb
   handMovement->fingerMovement[0] = new FingerMovement(10, 300);
   handMovement->fingerMovement[1] = new FingerMovement(0, 0);
   handMovement->fingerMovement[2] = new FingerMovement(0, 0);
   handMovement->fingerMovement[3] = new FingerMovement(0, 0);
   handMovement->fingerMovement[4] = new FingerMovement(10, 0);
   return handMovement;
}

HandMovement* HandMovementFactory::four() {
   Serial.println("Move to Four");
   HandMovement *handMovement = new HandMovement(hand);
   // Thumb
   handMovement->fingerMovement[0] = new FingerMovement(10, 300);
   handMovement->fingerMovement[1] = new FingerMovement(0, 0);
   handMovement->fingerMovement[2] = new FingerMovement(0, 0);
   handMovement->fingerMovement[3] = new FingerMovement(0, 0);
   handMovement->fingerMovement[4] = new FingerMovement(0, 0);
   return handMovement;
}

HandMovement* HandMovementFactory::five() {
   Serial.println("Move to Five");
   HandMovement *handMovement = new HandMovement(hand);
   // Thumb
   handMovement->fingerMovement[0] = new FingerMovement(0, 0);
   handMovement->fingerMovement[1] = new FingerMovement(0, 0);
   handMovement->fingerMovement[2] = new FingerMovement(0, 0);
   handMovement->fingerMovement[3] = new FingerMovement(0, 0);
   handMovement->fingerMovement[4] = new FingerMovement(0, 0);
   return handMovement;

}

HandMovement* HandMovementFactory::closePinch() {
   Serial.println("Move to closePinch");
   HandMovement *handMovement = new HandMovement(hand);
   // Thumb
   handMovement->fingerMovement[0] = new FingerMovement(9.2, 0);
   handMovement->fingerMovement[1] = new FingerMovement(7, 0);
   handMovement->fingerMovement[2] = new FingerMovement(10, 0);
   handMovement->fingerMovement[3] = new FingerMovement(10, 0);
   handMovement->fingerMovement[4] = new FingerMovement(10, 0);
   return handMovement;
}

HandMovement* HandMovementFactory::openPinch() {
   Serial.println("Move to closePinch");
   HandMovement *handMovement = new HandMovement(hand);
   // Thumb
   handMovement->fingerMovement[0] = new FingerMovement(9.2, 0);
   handMovement->fingerMovement[1] = new FingerMovement(7, 0);
   handMovement->fingerMovement[2] = new FingerMovement(0, 0);
   handMovement->fingerMovement[3] = new FingerMovement(0, 0);
   handMovement->fingerMovement[4] = new FingerMovement(0, 0);
   return handMovement;
}

HandMovement* HandMovementFactory::idle() {
   Serial.println("Move to Idle");
   HandMovement *handMovement = new HandMovement(hand);
   // Thumb
   handMovement->fingerMovement[0] = new FingerMovement(1, 0);
   handMovement->fingerMovement[1] = new FingerMovement(1, 0);
   handMovement->fingerMovement[2] = new FingerMovement(2, 0);
   handMovement->fingerMovement[3] = new FingerMovement(1, 0);
   handMovement->fingerMovement[4] = new FingerMovement(1, 0);
   return handMovement;
}

HandMovement* HandMovementFactory::point() {
   Serial.println("Move to Point");
   return one(true);
}