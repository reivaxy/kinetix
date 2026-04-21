
#include "MessageProcessor.h"
#include "BtServer.h"

MessageProcessor::MessageProcessor(Hand *hand, Settings *settings, Display *display, JsonDocument &systemInfo) {
   this->hand = hand;
   this->settings = settings;
   this->display = display;
   this->systemInfo = systemInfo;
   handMovement = NULL;
   hmf = new HandMovementFactory(hand);
}

void MessageProcessor::setBtServer(BtServer *btServer) {
   this->btServer = btServer;
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

      case systemConfig:
         log_i("Processing write system config message");
         {
            // Handle device name updates: format is s_deviceName=NewName
            char* equalSign = strchr(message, '=');
            if (equalSign != NULL) {
               *equalSign = 0;  // terminate key string
               const char* key = message;
               const char* value = equalSign + 1;
               
               if (strcmp(key, "s_deviceName") == 0) {
                  // Only update if value is not empty
                  if (strlen(value) > 0) {
                     settings->setDeviceName(value);
                     log_i("Device name updated to: %s, restart to apply changes.", value);

                  } else {
                     log_w("Device name cannot be empty, ignoring update");
                  }
               }
            } else {
               log_i("Invalid system config format, expected key=value");
            }
         }
         break;

      case password:
         log_i("Processing password message");
         {
            // Parse JSON payload with pwdCheck, newPwd, and resetPwd fields
            JsonDocument pwdDoc;
            DeserializationError error = deserializeJson(pwdDoc, message);
            
            if (error) {
               log_w("Invalid JSON in password payload: %s", error.c_str());
               if (btServer != nullptr) {
                  btServer->disableAllCharacteristics();
               }
               break;
            }
            
            // Get current stored password
            String storedPassword = settings->getPassword();
            
            // Extract pwdCheck field
            const char* pwdCheck = pwdDoc["pwdCheck"] | "";
            
            // Validate the current password
            if (strcmp(pwdCheck, storedPassword.c_str()) != 0) {
               log_w("Password check failed");
               if (btServer != nullptr) {
                  btServer->disableAllCharacteristics();
               }
               display->setLine(CONNECTED_DISPLAY_LINE, "Auth: FAIL");
               break;
            }
            
            log_i("Password check passed");
            
            // Check if newPwd is provided and set it
            if (pwdDoc["newPwd"].is<const char*>()) {
               const char* newPwd = pwdDoc["newPwd"] | "";
               settings->setPassword(newPwd);
               log_i("Password updated");
               display->setLine(CONNECTED_DISPLAY_LINE, "Pwd: Changed");
            }
            
            // Check if resetPwd is true and reset the password
            if (pwdDoc["resetPwd"] | false) {
               settings->setPassword("");
               log_i("Password reset to empty");
               display->setLine(CONNECTED_DISPLAY_LINE, "Pwd: Reset");
            }
            
            // Authenticate the client
            if (btServer != nullptr) {
               btServer->setClientAuthenticated(true);
            }
            display->setLine(CONNECTED_DISPLAY_LINE, "Auth: OK");
         }
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
      // Ensure device name is always included
      systemInfo["deviceName"] = settings->getDeviceName().c_str();
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

   case password:
      log_i("Processing read password authentication state");
      {
         // Return current authentication status
         bool authenticated = (btServer != nullptr) ? btServer->isClientAuthenticated() : false;
         const char* response = authenticated ? "true" : "false";
         log_i("Password authentication response: %s", response);
         characteristic->setValue(response);
      }
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
   hand->stop();

   if (handMovement != NULL) {
     handMovement->stop();
     delete(handMovement);
     handMovement = NULL;
   }
   if (seq != NULL) {
      seq->stop();
      delete(seq);
      seq = NULL;
   }

   if (0 == strcmp(movementName, "calibration")) {
      calibration();
      return;
   }
   if (0 == strcmp(movementName, "scratch")) {
      scratch();
      return;
   }

   if (0 == strcmp(movementName, "come")) {
      come();
      return;
   }

   if (0 == strcmp(movementName, "demo")) {
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