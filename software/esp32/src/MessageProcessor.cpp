
#include "MessageProcessor.h"


MessageProcessor::MessageProcessor(Hand *hand) {
   this->hand = hand;
   handMovement = NULL;
   hmf = new HandMovementFactory(hand);
}


// TODO: handle a FIFO stack of messages ?
void MessageProcessor::processMessage(MessageType type, char* message) {
   log_i("Processing message type '%d': '%s'", type, message);
   switch (type) {
   case movement:
      startMovement(message);
      break;
   
   default:
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