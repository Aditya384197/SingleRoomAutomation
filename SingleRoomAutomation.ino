/*
  SingleRoomAutomation.ino
  Single-room ESP32 automation - Fan + LED, 2-relay standalone.

  Hardware:
    Fan relay  -> GPIO26 (active LOW)
    LED relay  -> GPIO27 (active LOW)
    Fan switch -> GPIO32 (maintained toggle, to GND, INPUT_PULLUP)
    LED switch -> GPIO33 (maintained toggle, to GND, INPUT_PULLUP)

  WiFi: connects directly using WIFI_SSID/WIFI_PASSWORD from Config.h -
  no setup AP, no captive portal. Check the Serial Monitor (115200 baud)
  for the IP address once connected, and use that to reach the dashboard
  in a browser. The dashboard itself still has a WiFi Client page (under
  Settings) to switch to a different network later without reflashing.

  Required libraries (install via Arduino IDE Library Manager):
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

#include "Config.h"
#include "StorageManager.h"
#include "TimeManager.h"
#include "DeviceControl.h"
#include "ScheduleManager.h"
#include "TimerManager.h"
#include "WebDashboard.h"

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

static bool wasConnected = false;
static unsigned long lastReconnectAttemptMs = 0;
static uint8_t consecutiveFailedAttempts = 0;

static void wifi_maintain() {
  if (WiFi.status() == WL_CONNECTED) {
    consecutiveFailedAttempts = 0;
    if (!wasConnected) {
      wasConnected = true;
      webDashboard_startMdns();
      Serial.println("[WIFI] Connected. IP address: " + WiFi.localIP().toString());
      Serial.println("[WIFI] Open that address in a browser to reach the dashboard.");
    }
    return;
  }

  if (wasConnected) {
    wasConnected = false;
    Serial.println("[WIFI] Connection lost - retrying...");
  }

  unsigned long now = millis();
  if (now - lastReconnectAttemptMs < WIFI_RECONNECT_CHECK_MS) return;
  lastReconnectAttemptMs = now;

  consecutiveFailedAttempts++;
  if (consecutiveFailedAttempts >= WIFI_RECONNECT_HARD_RETRY_AFTER) {
    consecutiveFailedAttempts = 0;
    WiFi.disconnect(false, false);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
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

  // Dashboard server starts immediately - it doesn't need WiFi to be up
  // yet, it just won't be reachable until an IP exists.
  webDashboard_begin();

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("[WIFI] Connecting to " + String(WIFI_SSID) + "...");
}

void loop() {
  deviceControl_loop();
  scheduleManager_loop();
  timerManager_loop();
  timeManager_loop();

  wifi_maintain();
  webDashboard_loop();

  statusLed_update();
}
