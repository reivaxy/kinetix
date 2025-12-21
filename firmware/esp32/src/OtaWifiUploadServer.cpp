#include "OtaWifiUploadServer.h"

#include <WiFi.h>
#include <Update.h>

static String makeToken()
{
  uint32_t r1 = (uint32_t)esp_random();
  uint32_t r2 = (uint32_t)esp_random();
  char buf[17];
  snprintf(buf, sizeof(buf), "%08lx%08lx", (unsigned long)r1, (unsigned long)r2);
  return String(buf);
}

static String trimCopy(const String &s)
{
  String t = s;
  t.trim();
  return t;
}

OtaWifiUploadServer::OtaWifiUploadServer(uint16_t port, ReplyFn reply)
    : _port(port), _server(port), _reply(std::move(reply)) {}

void OtaWifiUploadServer::setReply(ReplyFn reply) { _reply = std::move(reply); }

void OtaWifiUploadServer::replyLine(const String &s)
{
  if (_reply)
    _reply(s);
}

void OtaWifiUploadServer::setState(State s, const String &msg)
{
  _state = s;
  _stateMsg = msg;

  String st;
  switch (_state)
  {
  case State::Idle:
    st = "IDLE";
    break;
  case State::ConnectingWifi:
    st = "CONNECTING_WIFI";
    break;
  case State::Ready:
    st = "READY";
    break;
  case State::Error:
    st = "ERROR";
    break;
  }
  if (_stateMsg.length())
    st += " - " + _stateMsg;
  replyLine("[OTA] " + st);
}

bool OtaWifiUploadServer::isRunning() const { return _state == State::Ready; }
String OtaWifiUploadServer::url() const { return _url; }

bool OtaWifiUploadServer::parseStartArgs(const String &cmd, String &ssidOut, String &passOut)
{
  String c = cmd;
  c.trim();

  int sp = c.indexOf(' ');
  String head = (sp < 0) ? c : c.substring(0, sp);
  head.toUpperCase();
  if (head != "OTA_START")
    return false;

  ssidOut = "";
  passOut = "";
  if (sp < 0)
    return true;

  String args = trimCopy(c.substring(sp + 1));
  if (args.length() == 0)
    return true;

  int p1 = args.indexOf(';');
  if (p1 < 0)
    return true; // treat as "no creds"
  ssidOut = trimCopy(args.substring(0, p1));
  passOut = trimCopy(args.substring(p1 + 1));
  return true;
}

// IMPORTANT: This must NOT be called from a BLE callback context.
// It is called from taskMain() only.
bool OtaWifiUploadServer::ensureWifi(const String &ssid, const String &pass)
{
  if (WiFi.status() == WL_CONNECTED)
  {
    replyLine("[OTA] WiFi already connected, IP=" + WiFi.localIP().toString());
    return true;
  }

  if (ssid.isEmpty())
  {
    setState(State::Error, "WiFi not connected and no SSID provided");
    return false;
  }

  WiFi.mode(WIFI_STA);
  WiFi.persistent(false);
  delay(50);

  WiFi.onEvent([&](WiFiEvent_t event, WiFiEventInfo_t info) {
    switch (event) {
      case ARDUINO_EVENT_WIFI_STA_START:
        Serial.println("[WIFI] STA_START");
        break;
      case ARDUINO_EVENT_WIFI_STA_CONNECTED:
        Serial.println("[WIFI] CONNECTED to AP");
        break;
      case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        Serial.print("[WIFI] GOT_IP: ");
        Serial.println(WiFi.localIP());
        break;
      case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        Serial.printf("[WIFI] DISCONNECTED reason=%d\n", info.wifi_sta_disconnected.reason);
        WiFi.disconnect(true);
        this->_stopTask = true;
        break;
      default:
        break;
    } 
  });

  setState(State::ConnectingWifi, "WiFi.begin(" + ssid + ")");
  WiFi.begin(ssid.c_str(), pass.c_str());

  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED)
  {
    if (_stopTask)
    {
      setState(State::Idle, "stopped");
      return false;
    }
    if (millis() - start > 30000)
    {
      setState(State::Error, "WiFi connect timeout");
      return false;
    }
    delay(200);
  }

  replyLine("[OTA] WiFi connected, IP=" + WiFi.localIP().toString());
  return true;
}

void OtaWifiUploadServer::installRoutes()
{
  static const char* kHeaders[] = { "Content-Length", "X-OTA-Token", "Content-Type" };
  _server.collectHeaders(kHeaders, 3);
  _server.on("/", HTTP_GET, [this]()
             { handleRoot(); });

  _server.on(
      "/update",
      HTTP_POST,
      [this]()
      { handleUpdate(); },
      [this]()
      { handleUpdateUpload(); });

  _server.onNotFound([this]()
                     { _server.send(404, "text/plain", "Not Found"); });
}

void OtaWifiUploadServer::handleRoot()
{
  _server.send(200, "text/plain",
               "ESP32 OTA upload server running.\n"
               "POST /update?token=... with Content-Type: multipart/form-data\n");
}

static uint32_t nowMs() { return (uint32_t)millis(); }

void OtaWifiUploadServer::noteActivity()
{
  _lastActivityMs = nowMs();
}

void OtaWifiUploadServer::maybeSendProgress(bool force)
{
  if (!_uploading)
    return;

  uint32_t t = nowMs();
  if (!force && (t - _lastProgressMs) < OTA_PROGRESS_MIN_MS)
    return;

  uint8_t pct = 0;
  if (_expectedSize > 0)
  {
    pct = (uint8_t)((_bytesReceived * 100ULL) / _expectedSize);
    if (pct > 100)
      pct = 100;
  }
  else
  {
    // Unknown total size; report bytes only
    pct = 0;
  }

  // Throttle by percent steps as well
  if (!force && _expectedSize > 0)
  {
    if (_lastPctSent != 255 && (pct < _lastPctSent + OTA_PROGRESS_STEP_PCT))
      return;
  }

  _lastProgressMs = t;
  _lastPctSent = pct;

  if (_expectedSize > 0)
  {
    replyLine("OTA_PROGRESS " + String(pct) + " " +
              String(_bytesReceived) + "/" + String(_expectedSize));
  }
  else
  {
    replyLine("OTA_PROGRESS_BYTES " + String(_bytesReceived));
  }
}

void OtaWifiUploadServer::handleUpdateUpload()
{
  String token = _server.header("X-OTA-Token");

  if (token != _token) {
      _server.send(403, "text/plain", "Forbidden");
      return;
  }

  HTTPUpload &upload = _server.upload();

  noteActivity();

  if (upload.status == UPLOAD_FILE_START)
  {
    _uploading = true;
    _bytesReceived = 0;
    _expectedSize = 0;
    _lastPctSent = 255;
    _lastProgressMs = 0;

    // Try to read Content-Length from request (nice for percent progress)
    String cl = _server.header("Content-Length");
    if (cl.length())
    {
      _expectedSize = (size_t)cl.toInt();
    }

    replyLine("[OTA] upload start" + String(_expectedSize ? (" total=" + String(_expectedSize)) : ""));

    if (!Update.begin(UPDATE_SIZE_UNKNOWN))
    {
      replyLine("[OTA] Update.begin failed");
    }

    // Send initial progress
    maybeSendProgress(true);
  }
  else if (upload.status == UPLOAD_FILE_WRITE)
  {
    _bytesReceived += upload.currentSize;

    size_t written = Update.write(upload.buf, upload.currentSize);
    if (written != upload.currentSize)
    {
      replyLine("[OTA] Update.write failed");
    }

    // Progress update (throttled)
    maybeSendProgress(false);
  }
  else if (upload.status == UPLOAD_FILE_END)
  {
    // Final progress update
    maybeSendProgress(true);

    _uploading = false;

    if (Update.end(true))
    {
      replyLine("[OTA] upload complete, size=" + String(upload.totalSize));
      replyLine("OTA_DONE");
    }
    else
    {
      replyLine(String("[OTA] Update.end failed: ") + Update.errorString());
      replyLine("OTA_ERROR");
    }
  }
  else if (upload.status == UPLOAD_FILE_ABORTED)
  {
    Update.abort();
    _uploading = false;
    replyLine("[OTA] upload aborted");
    replyLine("OTA_ABORTED");
  }
}

void OtaWifiUploadServer::handleUpdate()
{
  String token = _server.header("X-OTA-Token");
  log_i("Received token: %s", token.c_str());
  
  if (token != _token)
  {
    _server.send(403, "text/plain", "Forbidden");
    return;
  }

  if (Update.hasError())
  {
    _server.send(500, "text/plain", "Update failed");
    // Stay alive briefly so phone can receive OTA_ERROR
    return;
  }

  _server.send(200, "text/plain", "OK, rebooting");
  replyLine("[OTA] rebooting");
  delay(200);
  ESP.restart();
}

void OtaWifiUploadServer::taskTrampoline(void *arg)
{
  static_cast<OtaWifiUploadServer *>(arg)->taskMain();
  // taskMain() deletes the task (or returns and we delete here)
  vTaskDelete(nullptr);
}

void OtaWifiUploadServer::taskMain()
{
  // ---- WiFi + server start happens HERE (safe task context) ----
  if (!ensureWifi(_pendingSsid, _pendingPass))
  {
    _task = nullptr;
    return;
  }

  _token = makeToken();
   installRoutes();
  _server.begin();
  IPAddress ip = WiFi.localIP();
  _url = "http://" + ip.toString() + ":" + String(_port) + "/update?token=" + _token;

  setState(State::Ready, "listening :" + String(_port));
  replyLine("OTA_READY " + _url);

  // ---- Serve HTTP until stopped ----
  _lastActivityMs = nowMs();
  uint32_t sessionStart = _lastActivityMs;

  while (!_stopTask)
  {
    _server.handleClient();
    vTaskDelay(pdMS_TO_TICKS(5));

    uint32_t t = nowMs();

    // Session timeout: stop even if someone keeps poking it
    if ((t - sessionStart) > OTA_SESSION_TIMEOUT_MS)
    {
      replyLine("OTA_TIMEOUT session");
      _stopTask = true;
      break;
    }

    // Idle timeout: no activity and not currently uploading
    if (!_uploading && (t - _lastActivityMs) > OTA_IDLE_TIMEOUT_MS)
    {
      replyLine("OTA_TIMEOUT idle");
      _stopTask = true;
      break;
    }
  }

  log_i("Stopping server");
  // Stop server
  _server.stop();
  _token = "";
  _url = "";
  _uploading = false;
  _bytesReceived = 0;
  _expectedSize = 0;

  setState(State::Idle, "stopped");
  _task = nullptr;
}

bool OtaWifiUploadServer::start(const String &ssid, const String &pass)
{
  if (_task)
  {
    replyLine("[OTA] already running");
    return false;
  }

  // Store credentials and start the worker task.
  // IMPORTANT: do NOT call WiFi.* here; start() may be invoked from BLE callback.
  _pendingSsid = ssid;
  _pendingPass = pass;

  _stopTask = false;
  setState(State::ConnectingWifi, "scheduled");

  BaseType_t ok = xTaskCreatePinnedToCore(
      taskTrampoline,
      "ota_http",
      6144,
      this,
      1,
      &_task,
      1);

  if (ok != pdPASS)
  {
    _task = nullptr;
    setState(State::Error, "failed to create task");
    return false;
  }

  return true;
}

void OtaWifiUploadServer::stop()
{
  if (_task)
  {
    _stopTask = true;
    delay(20); // allow task loop to exit
    // task will clear _task when it exits
  }
  else
  {
    // ensure consistent state
    _server.stop();
    _token = "";
    _url = "";
    setState(State::Idle, "stopped");
  }
}

bool OtaWifiUploadServer::handleCommand(const String &cmd)
{
  String c = cmd;
  c.trim();
  if (c.isEmpty())
    return false;

  String up = c;
  up.toUpperCase();

  if (up == "OTA_STATUS")
  {
    replyLine("OTA_STATUS " + String(isRunning() ? "RUNNING" : "STOPPED"));
    if (_url.length())
      replyLine("OTA_URL " + _url);
    return true;
  }

  if (up == "OTA_STOP")
  {
    stop();
    return true;
  }

  String ssid, pass;
  if (!parseStartArgs(c, ssid, pass))
    return false;

  // OTA_START (with/without creds)
  (void)start(ssid, pass);
  return true;
}
