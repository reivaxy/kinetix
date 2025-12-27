
#include "Settings.h"

Settings::Settings() {
  preferences.begin("settings", false);
  loadSettings();
} 

void Settings::loadSettings() {
  String json = preferences.getString("settingsJson", "{}");
  DeserializationError error = deserializeJson(doc, json);
  if (error) {
    log_i("Failed to parse settings JSON, using defaults");
    doc.clear();
  } else {
    log_i("Settings loaded from JSON %s", json.c_str());
  }
}

String Settings::getSettingJson() {
  String json;
  serializeJson(doc, json);
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
              doc[key] = boolValue;
              log_i("Stored boolean setting");
           }
           break;
        default:
           log_i("Unknown setting type for key '%s'", key);
           return;
     }
     saveSettings();
     
  } else {
     log_i("Invalid setting format, expected key=value");
  }
}

void Settings::saveSettings() {
   String json;
   serializeJson(doc, json);
   preferences.putString("settingsJson", json);
   log_i("Settings saved to JSON");
}

int Settings::getInt(const char* key, int defaultValue) {
   if (doc[key].is<int>()) {
      return doc[key];
   }
   return defaultValue;
}
void Settings::setInt(const char* key, int value) {
   doc[key] = value;
}
void Settings::setString(const char* key, const char* value) {
   doc[key] = value;
}
const char* Settings::getString(const char* key, const char* defaultValue) {
   if (doc[key].is<const char*>()) {
      return doc[key];
   }
   return defaultValue;
}

void Settings::setBool(const char* key, boolean value) {
   doc[key] = value;
}
boolean Settings::getBool(const char* key, boolean defaultValue) {
   if (doc[key].is<bool>()) {
      return doc[key];
   }
   return defaultValue;
}

// close the preferences when done
Settings::~Settings() {
  saveSettings();
  preferences.end();
}
