/*
  SingleRoomAutomation.ino
  Single-room ESP32 automation - Fan + LED, 2-relay standalone.

  Hardware:
    Fan relay  -> GPIO26 (active LOW)
    LED relay  -> GPIO27 (active LOW)
    Fan switch -> GPIO32 (maintained toggle, to GND, INPUT_PULLUP)
    LED switch -> GPIO33 (maintained toggle, to GND, INPUT_PULLUP)

  Required libraries (install via Arduino IDE Library Manager):
    WiFiManager (by tzapu) - any reasonably recent version (2020+)
    ESPAsyncWebServer - use the actively maintained ESP32Async fork
      (https://github.com/ESP32Async/ESPAsyncWebServer), not the old
      unmaintained me-no-dev original.
    AsyncTCP - matching ESP32Async/AsyncTCP fork (same publisher as above)
    ArduinoJson - either v6.x or v7.x works, code auto-detects the
      installed version (see the ARDUINOJSON_VERSION_MAJOR check in
      WebDashboard.cpp)

  Board: ESP32 Dev Module. Compiles on Arduino-ESP32 core 2.x and 3.x.
*/

#include <WiFi.h>
#include <WiFiManager.h>
#include <string.h>

#include "Config.h"
#include "StorageManager.h"
#include "TimeManager.h"
#include "DeviceControl.h"
#include "ScheduleManager.h"
#include "TimerManager.h"
#include "WebDashboard.h"

static WiFiManager wm;

static unsigned long lastBlinkMs = 0;
static bool statusLedState = false;

static void statusLed_update() {
  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(STATUS_LED_PIN, HIGH);
    return;
  }
  unsigned long now = millis();
  if (now - lastBlinkMs > 500UL) {
    lastBlinkMs = now;
    statusLedState = !statusLedState;
    digitalWrite(STATUS_LED_PIN, statusLedState ? HIGH : LOW);
  }
}

static unsigned long lastReconnectAttemptMs = 0;
static uint8_t consecutiveFailedAttempts = 0;
static bool dashboardStarted = false;

static void wifi_maintain() {
  wm.process();

  if (WiFi.status() == WL_CONNECTED) {
    consecutiveFailedAttempts = 0;
    if (!dashboardStarted) {
      webDashboard_begin();
      dashboardStarted = true;
      Serial.println("[WIFI] Connected: " + WiFi.localIP().toString());
    }
    return;
  }

  unsigned long now = millis();
  if (now - lastReconnectAttemptMs < WIFI_RECONNECT_CHECK_MS) return;
  lastReconnectAttemptMs = now;

  consecutiveFailedAttempts++;
  if (consecutiveFailedAttempts >= WIFI_RECONNECT_HARD_RETRY_AFTER) {
    consecutiveFailedAttempts = 0;
    WiFi.disconnect(false, false);
    WiFi.begin();
  } else {
    WiFi.reconnect();
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, LOW);

  storage_begin();
  deviceControl_begin();
  timeManager_begin();
  scheduleManager_begin();
  timerManager_begin();

  ApConfig apCfg = storage_loadApConfig();

  wm.setConfigPortalBlocking(false);
  wm.setConfigPortalTimeout(WIFI_PORTAL_TIMEOUT_SEC);
  wm.setConnectTimeout(WIFI_CONNECT_TIMEOUT_SEC);

  if (strlen(apCfg.pass) > 0) {
    wm.autoConnect(apCfg.ssid, apCfg.pass);
  } else {
    wm.autoConnect(apCfg.ssid);
  }
}

void loop() {
  deviceControl_loop();
  scheduleManager_loop();
  timerManager_loop();
  timeManager_loop();

  wifi_maintain();
  if (dashboardStarted) {
    webDashboard_loop();
  }

  statusLed_update();
}
