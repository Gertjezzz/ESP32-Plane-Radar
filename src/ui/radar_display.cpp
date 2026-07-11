#include "ui/radar_theme.h"
#include "services/adsb_client.h"
#include <TFT_eSPI.h>
#include <cmath>

extern bool radarShowHelicopters;

namespace ui::radar {

static TFT_eSPI tft;

static void drawHelicopterIcon(int x, int y, uint16_t color) {
  tft.drawCircle(x, y, 4, color);
  tft.drawLine(x - 3, y, x + 3, y, color);
  tft.drawLine(x, y - 3, x, y + 3, color);
}

static void drawAirplaneIcon(int x, int y, float heading, uint16_t color) {
  float rad = heading * M_PI / 180.0f;
  int x1 = x + 6 * cos(rad);
  int y1 = y - 6 * sin(rad);
  int x2 = x - 3 * cos(rad + 0.5f);
  int y2 = y + 3 * sin(rad + 0.5f);
  int x3 = x - 3 * cos(rad - 0.5f);
  int y3 = y + 3 * sin(rad - 0.5f);
  tft.fillTriangle(x1, y1, x2, y2, x3, y3, color);
}

static void drawSpeedVector(int x, int y, float speed, float heading, uint16_t color) {
  float rad = heading * M_PI / 180.0f;
  int len = static_cast<int>(speed / 20.0f);
  int x2 = x + len * cos(rad);
  int y2 = y - len * sin(rad);
  tft.drawLine(x, y, x2, y2, color);
}

void drawAircraftSymbol(const services::adsb::Aircraft& ac, int x, int y) {
  if (ac.kind == services::adsb::AircraftKind::Helicopter && !radarShowHelicopters)
    return;

  uint16_t symbolColor =
      (ac.kind == services::adsb::AircraftKind::Helicopter)
          ? kRadarTheme.helicopterSymbol
          : kRadarTheme.airplaneSymbol;

  uint16_t vectorColor =
      (ac.kind == services::adsb::AircraftKind::Helicopter)
          ? kRadarTheme.helicopterVector
          : kRadarTheme.airplaneVector;

  if (ac.kind == services::adsb::AircraftKind::Helicopter)
    drawHelicopterIcon(x, y, symbolColor);
  else
    drawAirplaneIcon(x, y, ac.track_deg, symbolColor);

  drawSpeedVector(x, y, ac.gs_knots, ac.track_deg, vectorColor);
}

void renderRadar(const services::adsb::Aircraft* list, size_t count) {
  tft.fillScreen(kRadarTheme.background);

  for (size_t i = 0; i < count; ++i) {
    const auto& ac = list[i];
    int x = static_cast<int>(ac.lon * 2);
    int y = static_cast<int>(ac.lat * 2);
    drawAircraftSymbol(ac, x, y);
  }
}

}  // namespace ui::radar
