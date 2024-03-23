#include <Arduino.h>

#include "Hand.h"
#include "HandMovementFactory.h"
#include "Sequence.h"


Hand *hand = NULL;

int start = 0;
int finger = 0;
bool isClosed = true;

HandMovementFactory *hmf = NULL;
HandMovement *hm = NULL;
Sequence *seq = NULL;

void setup() {
  Serial.begin(115200);
  delay(5000);
  Serial.println("started");

  start = millis();
  isClosed = true;
  hand = new Hand();
  hmf = new HandMovementFactory(hand);
  seq = new Sequence(0); // 0 is repeat forever

  seq->addMovement(hmf->fist(), 4000);
  seq->addMovement(hmf->idle(), 4000);
  seq->addMovement(hmf->one());
  seq->addMovement(hmf->two());
  seq->addMovement(hmf->three());
  seq->addMovement(hmf->four());
  seq->addMovement(hmf->five());
  seq->addMovement(hmf->fu(), 2000);
  seq->addMovement(hmf->openPinch());
  seq->addMovement(hmf->ok());
  seq->addMovement(hmf->pointing());
  seq->addMovement(hmf->closePinch());
  seq->addMovement(hmf->idle(), 2000);
  
  // int f = 4;
  // int tempo = 2000;
  // seq->addMovement(hmf->closeFingerBy(f, 0), tempo);
  // seq->addMovement(hmf->closeFingerBy(f, 20), tempo);
  // seq->addMovement(hmf->closeFingerBy(f, 40), tempo);
  // seq->addMovement(hmf->closeFingerBy(f, 60), tempo);
  // seq->addMovement(hmf->closeFingerBy(f, 80), tempo);
  // seq->addMovement(hmf->closeFingerBy(f, 100), tempo);

  seq->start();

}

void loop() {
  seq->run();

}