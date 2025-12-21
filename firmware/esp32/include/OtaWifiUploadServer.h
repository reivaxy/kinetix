#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include <functional>
#include <esp_wifi.h>
#include <esp_event.h>

class OtaWifiUploadServer {
public:
  using ReplyFn = std::function<void(const String&)>;

  explicit OtaWifiUploadServer(uint16_t port = 8080, ReplyFn reply = nullptr);
  void setReply(ReplyFn reply);

  // Handle OTA commands. Returns true if the command was recognized (handled).
  //
  // Commands (case-insensitive):
  //   OTA_START                 -> starts server if WiFi already connected
  //   OTA_START <ssid>;<pass>   -> connect WiFi then start server
  //   OTA_STATUS
  //   OTA_STOP
  bool handleCommand(const String& cmd);

  bool start(const String& ssid = "", const String& pass = "");
  void stop();

  bool isRunning() const;
  String url() const;

private:
  enum class State : uint8_t { Idle, ConnectingWifi, Ready, Error };

  static void taskTrampoline(void* arg);
  void taskMain();

  void installRoutes();
  void handleRoot();
  void handleUpdate();         // final response
  void handleUpdateUpload();   // upload chunks
  void handleUpdateRaw();   // upload raw

  bool ensureWifi(const String& ssid, const String& pass);
  bool parseStartArgs(const String& cmd, String& ssidOut, String& passOut);

  void replyLine(const String& s);
  void setState(State s, const String& msg = "");
  void noteActivity();
  void maybeSendProgress(bool force);

  uint16_t _port;
  WebServer _server;

  ReplyFn _reply;
  State _state = State::Idle;
  String _stateMsg;

  String _token;
  String _url;

  TaskHandle_t _task = nullptr;
  volatile bool _stopTask = false;
  String _pendingPass;
  String _pendingSsid;

  // Progress / timeouts
  bool     _uploading = false;
  size_t   _bytesReceived = 0;
  size_t   _expectedSize = 0;      // from Content-Length if provided, else 0
  uint32_t _lastProgressMs = 0;
  uint32_t _lastActivityMs = 0;
  uint8_t  _lastPctSent = 255;

  static constexpr uint32_t OTA_IDLE_TIMEOUT_MS    = 60 * 1000;   // 60s no upload/data -> stop
  static constexpr uint32_t OTA_SESSION_TIMEOUT_MS = 2  * 60*1000; // 5min total -> stop
  static constexpr uint32_t OTA_PROGRESS_MIN_MS    = 300;         // throttle notifications
  static constexpr uint8_t  OTA_PROGRESS_STEP_PCT  = 2;           // send every 2%

  static constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 15000;
};
