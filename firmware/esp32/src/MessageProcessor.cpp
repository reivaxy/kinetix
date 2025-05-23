
#include "MessageProcessor.h"


MessageProcessor::MessageProcessor(Hand *hand) {
   this->hand = hand;
   handMovement = NULL;
   hmf = new HandMovementFactory(hand);
}


// TODO: handle a FIFO stack of messages ?
void MessageProcessor::processWriteMsg(MessageType type, char* message) {
   log_i("Processing write message type '%d': '%s'", type, message);
   switch (type) {
   case movement:
      startMovement(message);
      break;

   case config:
      log_i("Processing write config message");
      break;

   default:
      log_i("Message type %d has no write processing defined", type);
      break;
   }
}

void MessageProcessor::processReadMsg(MessageType type, char* message, BLECharacteristic *characteristic) {
   log_i("Processing read message type '%d': '%s'", type, message);
   switch (type) {

   case config:
      log_i("Processing read config message");
      #ifdef GIT_REV
      characteristic->setValue(GIT_REV);
      #endif
      break;

   default:
      log_i("Message type %d has no read processing defined", type);
      break;
   }
}

void MessageProcessor::run() {
   if (handMovement != NULL) {
      handMovement->run();
   }
   if (seq != NULL) {
      seq->run();
   }
}

void MessageProcessor::startMovement(char *movementName) {

   // Make sure we use the appropriate min and max Servo values when fingers are wired.
   hand->setCalibration(false);

   if (handMovement != NULL) {
     delete(handMovement);
     handMovement = NULL;
   }
   if (seq != NULL) {
      delete(seq);
      seq = NULL;
   }

   if (0 == strcmp(movementName, "calibration")) {
      hand->stop();
      calibration();
      return;
   }
   if (0 == strcmp(movementName, "scratch")) {
      hand->stop();
      scratch();
      return;
   }

   if (0 == strcmp(movementName, "come")) {
      hand->stop();
      come();
      return;
   }
   HandMovement *newHandMovement = hmf->getByName(movementName);
   if (newHandMovement != NULL) {
      hand->stop();
      handMovement = newHandMovement;  
      handMovement->start();
   }
}

void MessageProcessor::calibration() {
  log_i("Starting calibration sequence");  
  hand->setCalibration(true);  // This shouldn't be done once the finger are wired to the servos, may break.
  HandMovementFactory *calibrationHmf = new HandMovementFactory(hand);
  seq = new Sequence(0); // 0 is repeat forever
  seq->addMovement(calibrationHmf->five(), 5000);
  seq->addMovement(calibrationHmf->fist(), 1500);
  seq->start();
}

void MessageProcessor::scratch() {
  log_i("Starting scratch sequence");
  HandMovementFactory *hmf = new HandMovementFactory(hand);
  seq = new Sequence(0); // 0 is repeat forever
  seq->addMovement(hmf->scratchOpen(), 600);
  seq->addMovement(hmf->scratchClose(), 600);
  seq->start();
}

void MessageProcessor::come() {
  log_i("Starting come sequence");
  HandMovementFactory *hmf = new HandMovementFactory(hand);
  seq = new Sequence(5);
  seq->addMovement(hmf->comeOpen(), 500);
  seq->addMovement(hmf->comeClose(), 500);
  seq->start();
}