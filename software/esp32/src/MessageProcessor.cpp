
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
}

void MessageProcessor::startMovement(char *movementName) {
   if (handMovement != NULL) {
     hand->stop();
   //   delete(handMovement);
   }
   handMovement = hmf->getByName(movementName);
   if (handMovement != NULL) {
      handMovement->start();
   }
}