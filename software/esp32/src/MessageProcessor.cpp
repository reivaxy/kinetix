
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
   if (handMovement != NULL) {
     handMovement->stop();
     delete(handMovement);
     handMovement = NULL;
   }
   if (seq != NULL) {
      delete(seq);
      seq = NULL;
   }

   if (0 == strcmp(movementName, "calibration")) {
      calibration();
      return;
   }
   handMovement = hmf->getByName(movementName);
   if (handMovement != NULL) {
      handMovement->start();
   }
}

void MessageProcessor::calibration() {
  log_i("Starting calibration sequence.");
  Hand *hand = new Hand(true);
  HandMovementFactory *hmf = new HandMovementFactory(hand);
  seq = new Sequence(0); // 0 is repeat forever
  seq->addMovement(hmf->five(), 5000);
  seq->addMovement(hmf->fist(), 1000);
  seq->start();
}