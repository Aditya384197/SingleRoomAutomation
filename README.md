esp32-room-automation
ESP32-based single-room home automation — Fan + LED relay control with a live web dashboard, WebSocket push updates, daily schedules, one-shot timers, physical switch support, WiFiManager captive-portal setup, and OTA firmware updates.
Features
2-relay control (Fan, LED) — physical maintained-switch input and dashboard both supported, edge-triggered so they never fight or desync
Live dashboard over WebSocket (instant updates, no polling needed) with a 5s polling safety net
Daily ON/OFF schedule per device (handles overnight windows correctly)
Independent one-shot AUTO-ON / AUTO-OFF timers per device, reboot-safe via epoch time
NTP time sync with flash-persisted fallback (survives reboot without internet)
WiFiManager captive portal for first-time WiFi setup, plus in-dashboard WiFi scan/connect (including hidden networks)
OTA firmware updates with Basic-Auth and lockout-after-failed-attempts protection
Coalesced NVS writes — rapid switch flicking doesn't wear out flash
Build (Arduino core for ESP32, board: esp32:esp32:esp32)
Required libraries:
WiFiManager (tzapu)
ArduinoJson (v6 or v7)
AsyncTCP — ESP32Async fork
ESPAsyncWebServer — ESP32Async fork
arduino-cli compile --fqbn esp32:esp32:esp32 SingleRoomAutomation.ino
GitHub Actions (.github/workflows/build.yml) builds automatically on every push using arduino-cli.
Hardware
Function
GPIO
Fan relay (active LOW)
26
LED relay (active LOW)
27
Fan switch (to GND)
32
LED switch (to GND)
33
Status LED
2
Before deploying
Change the default secrets in Config.h:
API_SECRET_KEY
OTA_AUTH_PASS
