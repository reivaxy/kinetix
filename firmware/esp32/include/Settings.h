#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Preferences.h>

// Preferences keys mut be less than 15 characters
#define SETTINGS_NAMESPACE "settingsJson"
#define PASSWORD_NAMESPACE "pwd"
#define DEVICE_NAME_NAMESPACE "deviceName"
#ifdef LEFT_HAND
   #define POSITIONS_SETTING_NAMESPACE "leftPositions"
#else
   #define POSITIONS_SETTING_NAMESPACE "rightPositions"
#endif


class Settings {
public:
  Settings();
  ~Settings();
  void loadPreferences(const char* preferenceName, JsonDocument& jsonDoc);
  void savePreferences(const char* preferenceName, JsonDocument& jsonDoc);
  int getInt(const char* key, int defaultValue);
  void setInt(const char* key, int value);
  void setString(const char* key, const char* value);
  const char* getString(const char* key, const char* defaultValue);
  void setBool(const char* key, boolean value);
  boolean getBool(const char* key, boolean defaultValue);
  int getPosition(const char* key, int defaultValue);
  String getSettingJson();
  String getPositionsJson();
  String getPassword();
  void setPassword(const char* value);
  String getDeviceName();
  void setDeviceName(const char* value);

  void updateSetting(char* message);
  void updatePosition(char* message);

private:
  Preferences preferences; 
  JsonDocument settingsJsonDoc;
  JsonDocument positionsJsonDoc;

};
