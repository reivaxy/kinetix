
#include "MessageProcessor.h"


MessageProcessor::MessageProcessor(Hand *hand, Settings *settings, Display *display, JsonDocument &systemInfo) {
   this->hand = hand;
   this->settings = settings;
   this->display = display;
   this->systemInfo = systemInfo;
   handMovement = NULL;
   hmf = new HandMovementFactory(hand);
}


// TODO: handle a FIFO stack of messages ?
void MessageProcessor::processWriteMsg(MessageType type, char* message) {
   log_i("Processing write message type '%d': '%s'", type, message);
   String line;
   switch (type) {
      case movement:
         line = "Movement: " + String(message);
         display->setLine(MOVEMENT_DISPLAY_LINE, line.c_str());
         startMovement(message);
         break;

      case setting:
         log_i("Processing write setting message");
         settings->updateSetting(message);
         break;

      case positions:
         log_i("Processing write positions message");
         settings->updatePosition(message);
         hand->updateMaxPositionsFromSettings();
         break;

      default:
         log_i("Message type %d has no write processing defined", type);
         break;
   }
}

void MessageProcessor::processReadMsg(MessageType type, BLECharacteristic *characteristic) {
   log_i("Processing read message type '%d'", type);
   String config = "";
   String json;

   switch (type) {
   case systemConfig:
      log_i("Processing read systemConfig message");
      serializeJson(systemInfo, json);
      log_i("System info: %s", json.c_str());
      characteristic->setValue(json.c_str());
      break;

   case setting:
      log_i("Processing read setting message");
      // testing generic params
      characteristic->setValue(settings->getSettingJson().c_str() );         
      break;

   case positions:
      log_i("Processing read positions message");
      characteristic->setValue(settings->getPositionsJson().c_str() );         
      break;

   default:
      log_i("Message type %d has no read processing defined", type);
      break;
   }
}

void MessageProcessor::run() {
   if (seq != NULL) {
      seq->run();
   }
   hand->run();
}

void MessageProcessor::startMovement(char *movementName) {

   // Make sure we use the appropriate min and max Servo values when fingers are wired.
   // huh I can't remember why I call this here... 
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

   if (0 == strcmp(movementName, "demo")) {
      hand->stop();
      demo();
      return;
   }

   // Handle single finger open/close movements (oX and cX, where X is 0-4)
   if ((movementName[0] == 'o' || movementName[0] == 'c') && strlen(movementName) == 2 && movementName[1] >= '0' && movementName[1] <= '4') {
      int fingerIndex = movementName[1] - '0';
      int target = (movementName[0] == 'o') ? 0 : 100;
      hand->fingers[fingerIndex]->computeTarget(target);
      return;
   }


   HandMovement *newHandMovement = hmf->getByName(movementName);
   if (newHandMovement != NULL) {
      hand->stop();
      handMovement = newHandMovement;  
      handMovement->start();
   }
}

boolean MessageProcessor::isIdle() {
   if (handMovement != NULL) {
      return handMovement->isFinished();
   } else {
      return true;
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

Sequence* MessageProcessor::demo() {
  log_i("Starting demo sequence");
  HandMovementFactory *hmf = new HandMovementFactory(hand);
  seq = new Sequence(0);
  int delai = 1500;
  seq->addMovement(hmf->five(), delai*3);
  seq->addMovement(hmf->one(), delai);
  seq->addMovement(hmf->two(), delai);
  seq->addMovement(hmf->three(), delai);
  seq->addMovement(hmf->four(), delai);
  seq->addMovement(hmf->five(), delai);
  seq->addMovement(hmf->rock(), delai);
  seq->addMovement(hmf->love(), delai);
  seq->addMovement(hmf->fist(), delai);
  seq->addMovement(hmf->five(), 300);
  seq->addMovement(hmf->openPinch(), delai);
  seq->start();
  return seq;
}