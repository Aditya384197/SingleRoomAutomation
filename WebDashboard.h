#pragma once
/*
  WebDashboard.h
  ---------------
  ESPAsyncWebServer + WebSocket dashboard. Serves the HTML page, exposes
  a small REST API, and pushes live state to all connected browsers over
  WebSocket the instant anything changes (toggle, schedule fire, timer
  expiry, physical switch) - no polling needed on the client, though the
  page also polls every 5s as a safety net.
*/

#include <Arduino.h>

void webDashboard_begin();     // call once WiFi is up
void webDashboard_loop();      // call every loop() - cleans up dead WS clients
void webDashboard_broadcastState(); // push current status JSON to all clients
