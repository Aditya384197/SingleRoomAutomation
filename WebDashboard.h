#pragma once
/*
  WebDashboard.h
  ---------------
  ESPAsyncWebServer + WebSocket dashboard. Serves the HTML page, exposes
  a small REST API, and pushes live state to all connected browsers over
  WebSocket the instant anything changes (toggle, schedule fire, timer
  expiry, physical switch) - no polling needed on the client, though the
  page also polls every 5s as a safety net.

  webDashboard_begin() should be called once at boot. The device connects
  to WiFi directly (see Config.h) - reach the dashboard via its IP address
  (check Serial Monitor) or via smarthome.local once mDNS is up.
*/

#include <Arduino.h>

void webDashboard_begin();     // call once at boot
void webDashboard_loop();      // call every loop() - WS cleanup
void webDashboard_broadcastState(); // push current status JSON to all clients

// Call once, after WiFi actually connects.
void webDashboard_startMdns();
