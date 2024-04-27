#include <Arduino.h>

#include "Hand.h"
#include "HandMovementFactory.h"
#include "Sequence.h"

#include "CurrentMonitor.h"


Hand *hand = NULL;

int start = 0;
int finger = 0;
bool isClosed = true;

HandMovementFactory *hmf = NULL;
HandMovement *hm = NULL;
Sequence *seq = NULL;
CurrentMonitor curm;

void setup() {
  Serial.begin(115200);
  delay(5000);
  Serial.println("Setup");

  start = millis();
  isClosed = true;

  // curm.setPin(ADC1_CHANNEL_0);²

#ifdef HOME_SERVOS
  hand = new Hand();
  hmf = new HandMovementFactory(hand);
  seq = new Sequence(0); // 0 is repeat forever
  seq->addMovement(hmf->five(), 4000);
  seq->addMovement(hmf->fist(), 1000);
  seq->start();
#endif

#ifdef DEMO
  hand = new Hand();
  hmf = new HandMovementFactory(hand);
  seq = new Sequence(0); // 0 is repeat forever
  seq->addMovement(hmf->openPinch(), 4000);
  seq->addMovement(hmf->one());
  seq->addMovement(hmf->two());
  seq->addMovement(hmf->three());
  seq->addMovement(hmf->four());
  seq->addMovement(hmf->five());
  seq->start();
#endif

  // seq->addMovement(hmf->pointing());
  // seq->addMovement(hmf->closePinch());
  
  // // Testing intermediate positions of each finger one at a time
  // int f = 4;
  // int tempo = 4000;
  // seq->addMovement(hmf->closeFingerBy(f, 0), tempo);
  // seq->addMovement(hmf->closeFingerBy(f, 20), tempo);  // 20% closed
  // seq->addMovement(hmf->closeFingerBy(f, 40), tempo);
  // seq->addMovement(hmf->closeFingerBy(f, 60), tempo);
  // seq->addMovement(hmf->closeFingerBy(f, 80), tempo);   // 80% closed...
  // seq->addMovement(hmf->closeFingerBy(f, 100), tempo);

  // seq->addMovement(hmf->closeFingerBy(4, 0), 1000);
  // seq->addMovement(hmf->closeFingerBy(4, 100), 10000);
  // seq->addMovement(hmf->closeFingerBy(4, 50), 1000000);

  // seq->addMovement(hmf->five(), 4000);
  // seq->addMovement(hmf->fist());

}

void loop() {
  seq->run();
}

/**
 * @brief ADC tests on A0
 * 
 */
void readA0() {
  int r = curm.getValue();
  char msg[50];
  sprintf(msg, "Reading: %d", r);
  Serial.println(msg);
}