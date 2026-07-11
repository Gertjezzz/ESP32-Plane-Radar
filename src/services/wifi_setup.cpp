#include "services/wifi_setup.h"
#include "ui/radar_theme.h"
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>

using namespace ui::radar;

WebServer server(80);
Preferences prefs;

struct UserSettings {
  bool darkMode;
  bool showHelicopters;
  uint16_t airplaneColor;
  uint16_t helicopterColor;
};

UserSettings settings;
bool radarShowHelicopters = true;

static uint16_t parseHtmlColor(const String& s) {
  uint32_t rgb = strtol(s.substring(1).c_str(), nullptr, 16);
  uint8_t r = (rgb >> 16) & 0xFF;
  uint8_t g = (rgb >> 8) & 0xFF;
  uint8_t b = rgb & 0xFF;
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

static String toHtmlColor(uint16_t c) {
  uint8_t r = (c >> 8) & 0xF8;
  uint8_t g = (c >> 3) & 0xFC;
  uint8_t b = (c << 3) & 0xF8;
  char buf[10];
  snprintf(buf, sizeof(buf), "#%02X%02X%02X", r, g, b);
  return String(buf);
}

static void loadSettings() {
  prefs.begin("radar", true);
  settings.darkMode = prefs.getBool("dark", false);
  settings.showHelicopters = prefs.getBool("heli", true);
  settings.airplaneColor = prefs.getUShort("apc", TFT_RED);
  settings.helicopterColor = prefs.getUShort("hec", TFT_ORANGE);
  prefs.end();

  kRadarTheme.background = settings.darkMode ? TFT_BLACK : TFT_NAVY;
  kRadarTheme.airplaneSymbol = settings.airplaneColor;
  kRadarTheme.helicopterSymbol = settings.helicopterColor;
  radarShowHelicopters = settings.showHelicopters;
}

static void saveSettings() {
  prefs.begin("radar", false);
  prefs.putBool("dark", settings.darkMode);
  prefs.putBool("heli", settings.showHelicopters);
  prefs.putUShort("apc", settings.airplaneColor);
  prefs.putUShort("hec", settings.helicopterColor);
  prefs.end();
}

static void handleRoot() {
  String html =
    "<html><body>"
    "<h2>Plane Radar Settings</h2>"
    "<form method='POST' action='/save'>"

    "<label>Dark mode: "
    "<input type='checkbox' name='dark' " + String(settings.darkMode ? "checked" : "") + ">"
    "</label><br><br>"

    "<label>Show helicopters: "
    "<input type='checkbox' name='heli' " + String(settings.showHelicopters ? "checked" : "") + ">"
    "</label><br><br>"

    "<label>Airplane color: "
    "<input type='color' name='apc' value='" + toHtmlColor(settings.airplaneColor) + "'>"
    "</label><br><br>"

    "<label>Helicopter color: "
    "<input type='color' name='hec' value='" + toHtmlColor(settings.helicopterColor) + "'>"
    "</label><br><br>"

    "<input type='submit' value='Save'>"
    "</form>"
    "</body></html>";

  server.send(200, "text/html", html);
}

static void handleSave() {
  settings.darkMode = server.hasArg("dark");
  settings.showHelicopters = server.hasArg("heli");

  if (server.hasArg("apc"))
    settings.airplaneColor = parseHtmlColor(server.arg("apc"));

  if (server.hasArg("hec"))
    settings.helicopterColor = parseHtmlColor(server.arg("hec"));

  saveSettings();
  loadSettings();

  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "");
}

void wifiSetupBegin(const char* ssid, const char* pass) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
  }

  loadSettings();

  server.on("/", handleRoot);
  server.on("/save", handleSave);
  server.begin();
}

void wifiLoop() {
  server.handleClient();
}
