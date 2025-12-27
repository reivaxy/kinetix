#include <Arduino.h>

#include "Hand.h"
#include "HandMovementFactory.h"
#include "Sequence.h"
#include "BtServer.h"
#include "MessageProcessor.h"
#include "SensorProcessor.h"


#if defined DEMO
#define NEEDSEQ
#endif

// Display instanciation must not happen before setup so we will use a pointer and new...
#ifdef WITH_OLED_DISPLAY
RealDisplay *display;
#else
MockDisplay *display;
#endif


int start = 0;
int finger = 0;
bool isClosed = true;

Hand *hand = new Hand();
HandMovementFactory *hmf = new HandMovementFactory(hand);

BtServer *btServer = NULL;
MessageProcessor *messageProcessor = NULL;
SensorProcessor *sensorProcessor = NULL;
Sequence *seq = NULL;
Settings *settings;

void setup() {
  Serial.begin(115200);
  delay(3000);
  Serial.println("Setup");
  #ifdef GIT_REV
  log_i("Version %s\n", GIT_REV);
  #endif 

  #ifdef WITH_OLED_DISPLAY
  display = new RealDisplay();
  #else
  display = new MockDisplay();
  #endif
  display->setTitle("KinetiX");

  start = millis();
  isClosed = true;
 
  settings = new Settings();

  #ifdef SENSOR
  sensorProcessor = new SensorProcessor(hand, settings, display);
  log_i("With Sensor");
  #endif

  messageProcessor = new MessageProcessor(hand, settings, display);
  
  btServer = new BtServer(messageProcessor, display);
  #ifndef NEEDSEQ
  // Initialization sequence, do it just once
  seq = new Sequence(1); // this sequence runs just once
  seq->addMovement(hmf->five());
  seq->addMovement(hmf->half());
  seq->addMovement(hmf->five());
  log_i("Running init sequence");
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
  #ifndef NEEDSEQ
    messageProcessor->run();
  #endif
  #ifdef SENSOR
    if ((seq == NULL || !seq->isRunning())
          && messageProcessor->isIdle()) {
      sensorProcessor->run();
    }
  #endif
  if (seq != NULL) {
    seq->run();
  }
  display->refresh();
}
