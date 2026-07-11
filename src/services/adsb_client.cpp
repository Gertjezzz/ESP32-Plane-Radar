#include "services/adsb_client.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <cstring>

namespace services::adsb {

static Aircraft aircraft_buffer[kMaxAircraft];
static size_t aircraft_count = 0;
static PollFn poll_fn = nullptr;

size_t aircraftCount() {
  return aircraft_count;
}

const Aircraft* aircraftList() {
  return aircraft_buffer;
}

void setPollFn(PollFn fn) {
  poll_fn = fn;
}

static AircraftKind detectKind(const char* type) {
  if (type[0] == 'H') return AircraftKind::Helicopter;
  return AircraftKind::Airplane;
}

bool fetchUpdate(double center_lat, double center_lon, float fetch_radius_km) {
  HTTPClient http;

  char url[256];
  snprintf(
      url,
      sizeof(url),
      "https://opendata.adsb.fi/api/v2/aircraft?lat=%.6f&lon=%.6f&radius=%.1f",
      center_lat,
      center_lon,
      fetch_radius_km
  );

  http.begin(url);
  int code = http.GET();
  if (code != 200) {
    http.end();
    return false;
  }

  WiFiClient* stream = http.getStreamPtr();
  DynamicJsonDocument doc(200000);

  DeserializationError err = deserializeJson(doc, *stream);
  if (err) {
    http.end();
    return false;
  }

  aircraft_count = 0;

  JsonArray arr = doc.as<JsonArray>();
  for (JsonObject obj : arr) {
    if (aircraft_count >= kMaxAircraft) break;

    Aircraft& a = aircraft_buffer[aircraft_count];

    a.lat = obj["lat"] | 0.0f;
    a.lon = obj["lon"] | 0.0f;
    a.nose_deg = obj["heading"] | 0.0f;
    a.track_deg = obj["track"] | 0.0f;
    a.gs_knots = obj["speed"] | 0.0f;

    const char* cs = obj["callsign"] | "";
    const char* tp = obj["type"] | "";
    const char* al = obj["alt_baro"] | "";

    strncpy(a.callsign, cs, sizeof(a.callsign) - 1);
    a.callsign[sizeof(a.callsign) - 1] = '\0';

    strncpy(a.type, tp, sizeof(a.type) - 1);
    a.type[sizeof(a.type) - 1] = '\0';

    strncpy(a.alt, al, sizeof(a.alt) - 1);
    a.alt[sizeof(a.alt) - 1] = '\0';

    a.kind = detectKind(a.type);

    aircraft_count++;

    if (poll_fn) poll_fn();
  }

  http.end();
  return true;
}

}  // namespace services::adsb
