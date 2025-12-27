#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Preferences.h>

class Settings {
public:
  Settings();
  ~Settings();
  void loadSettings();
  void saveSettings();
  int getInt(const char* key, int defaultValue);
  void setInt(const char* key, int value);
  void setString(const char* key, const char* value);
  const char* getString(const char* key, const char* defaultValue);
  void setBool(const char* key, boolean value);
  boolean getBool(const char* key, boolean defaultValue);
  String getSettingJson();

  void updateSetting(char* message);

private:
  Preferences preferences; 
  JsonDocument doc;

};
