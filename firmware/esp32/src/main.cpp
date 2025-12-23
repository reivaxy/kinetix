#include <Arduino.h>

#include "Hand.h"
#include "HandMovementFactory.h"
#include "Sequence.h"
#include "BtServer.h"
#include "MessageProcessor.h"

// #include "CurrentMonitor.h"

#if defined HOME_SERVOS || defined DEMO
#define NEEDSEQ
#endif

int start = 0;
int finger = 0;
bool isClosed = true;

Hand *hand = new Hand();
HandMovementFactory *hmf = new HandMovementFactory(hand);

BtServer *btServer = NULL;
MessageProcessor *messageProcessor = NULL;
Sequence *seq = NULL;
Settings *settings;

// XOLEDDisplayClass *display;

void setup() {
  Serial.begin(115200);
  delay(3000);
  Serial.println("Setup");
  #ifdef GIT_REV
  log_i("Version %s\n", GIT_REV);
  #endif 

  // display = new XOLEDDisplayClass(0x3C, SDA, SCL, false, 250);
  // display->setTitle("KinetiX");

  start = millis();
  isClosed = true;
  settings = new Settings();
  messageProcessor = new MessageProcessor(hand, settings, NULL);
  btServer = new BtServer(messageProcessor);
  #ifndef NEEDSEQ
  // Initialization sequence, do it just once
  seq = new Sequence(1); // this sequence runs just once
  seq->addMovement(hmf->five());
  seq->addMovement(hmf->half());
  seq->addMovement(hmf->five());
  log_i("Running init sequence");
  seq->start();   
  #endif

  #ifdef HOME_SERVOS
  seq = new Sequence(0); // 0 is repeat forever
  seq->addMovement(hmf->five(), 5000);
  seq->addMovement(hmf->fist(), 1000);
  seq->start();
  #endif

  #ifdef DEMO
  seq = new Sequence(0); // 0 is repeat forever
  seq->addMovement(hmf->openPinch(), 4000);
  seq->addMovement(hmf->one());
  seq->addMovement(hmf->two());
  seq->addMovement(hmf->three());
  seq->addMovement(hmf->four());
  seq->addMovement(hmf->five());
  seq->start();
  #endif

}

void loop() {
  // display->refresh();
  #ifndef NEEDSEQ
  messageProcessor->run();
  #endif
  if (seq != NULL) {
    seq->run();
  }
}
