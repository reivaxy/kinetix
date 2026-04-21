
#include "Settings.h"

Settings::Settings() {
  preferences.begin("settings", false);
  loadPreferences(SETTINGS_NAMESPACE, settingsJsonDoc);
  loadPreferences(POSITIONS_SETTING_NAMESPACE, positionsJsonDoc);
  preferences.end();
}

void Settings::loadPreferences(const char* preferenceName, JsonDocument& jsonDoc) {
  String json = preferences.getString(preferenceName, "{}");
  DeserializationError error = deserializeJson(jsonDoc, json);
  if (error) {
    log_i("Failed to parse %s, using defaults", preferenceName);
    jsonDoc.clear();
  } else {
    log_i("%s loaded from JSON %s", preferenceName, json.c_str());
  }
}

String Settings::getSettingJson() {
  String json;
  serializeJson(settingsJsonDoc, json);
  return json;
}

String Settings::getPositionsJson() {
  String json;
  serializeJson(positionsJsonDoc, json);
  return json;
}

void Settings::updateSetting(char* message) {
  log_i("Updating setting %s", message);
  // Split on '='
  char* equalSign = strchr(message, '='); 
  if (equalSign != NULL) {
     *equalSign = 0; // terminate key string
     const char* key = message;
     const char* value = equalSign + 1;
     log_i("Setting key: '%s', value: '%s'", key, value);
     switch(key[0]) {
        case 'i': // integer
           {
              int intValue = atoi(value);
              setInt(key, intValue);
              log_i("Stored int setting");
           }
           break;
        case 's': // string
           setString(key, value);
           log_i("Stored string setting");
           break;
        case 'b': // boolean
           {
              bool boolValue = (strcmp(value, "true") == 0 || strcmp(value, "1") == 0);
              settingsJsonDoc[key] = boolValue;
              log_i("Stored boolean setting");
           }
           break;
        default:
           log_i("Unknown setting type for key '%s'", key);
           return;
     }
     savePreferences(SETTINGS_NAMESPACE, settingsJsonDoc);
     
  } else {
     log_i("Invalid setting format, expected key=value");
  }
}

void Settings::updatePosition(char* message) {
  log_i("Updating position %s", message);
  // Check if resetting positions
  if (strcmp(message, "reset") == 0) {
     log_i("Resetting all positions to factory defaults");
     positionsJsonDoc.clear();
     savePreferences(POSITIONS_SETTING_NAMESPACE, positionsJsonDoc);
     return;
  }
  // Split on '='
  char* equalSign = strchr(message, '=');
  if (equalSign != NULL) {
     *equalSign = 0; // terminate key string
     const char* key = message;
     const char* value = equalSign + 1;
     log_i("Position key: '%s', value: '%s'", key, value);
     int intValue = atoi(value);
     positionsJsonDoc[key] = intValue;
     log_i("Stored position");
     savePreferences(POSITIONS_SETTING_NAMESPACE, positionsJsonDoc);
  } else {
     log_i("Invalid position format, expected key=value");
  }
}

void Settings::savePreferences(const char* preferenceName, JsonDocument& jsonDoc) {
   preferences.begin("settings", false);
   String json;
   serializeJson(jsonDoc, json);
   preferences.putString(preferenceName, json);
   log_i("%s saved", preferenceName);
   preferences.end();
}

int Settings::getInt(const char* key, int defaultValue) {
   if (settingsJsonDoc[key].is<int>()) {
      return settingsJsonDoc[key];
   }
   return defaultValue;
}

void Settings::setInt(const char* key, int value) {
   settingsJsonDoc[key] = value;
}
void Settings::setString(const char* key, const char* value) {
   settingsJsonDoc[key] = value;
}
const char* Settings::getString(const char* key, const char* defaultValue) {
   if (settingsJsonDoc[key].is<const char*>()) {
      return settingsJsonDoc[key];
   }
   return defaultValue;
}

void Settings::setBool(const char* key, boolean value) {
   settingsJsonDoc[key] = value;
}
boolean Settings::getBool(const char* key, boolean defaultValue) {
   if (settingsJsonDoc[key].is<bool>()) {
      return settingsJsonDoc[key];
   }
   return defaultValue;
}

int Settings::getPosition(const char* key, int defaultValue) {
   if (positionsJsonDoc[key].is<int>()) {
      return positionsJsonDoc[key];
   }
   // Set the default value in the document
   positionsJsonDoc[key] = defaultValue;
    return defaultValue;
}

String Settings::getPassword() {
   preferences.begin("settings", false);
   String pwd = preferences.getString(PASSWORD_NAMESPACE, "");
   preferences.end();
   return pwd;
}

void Settings::setPassword(const char* value) {
   preferences.begin("settings", false);
   preferences.putString(PASSWORD_NAMESPACE, value);
   log_i("Password saved");
   preferences.end();
}

String Settings::getDeviceName() {
   preferences.begin("settings", false);
   String name = preferences.getString(DEVICE_NAME_NAMESPACE, "KinetiX");
   preferences.end();
   return name;
}

void Settings::setDeviceName(const char* value) {
   preferences.begin("settings", false);
   preferences.putString(DEVICE_NAME_NAMESPACE, value);
   log_i("Device name saved: %s", value);
   preferences.end();
}

// close the preferences when done
Settings::~Settings() {
  savePreferences(SETTINGS_NAMESPACE, settingsJsonDoc);
  savePreferences(POSITIONS_SETTING_NAMESPACE, positionsJsonDoc);
}
