#pragma once

#include <cstdint>

namespace ui::radar {

struct Theme {
  uint16_t background;
  uint16_t ring;
  uint16_t crosshair;

  uint16_t airplaneSymbol;
  uint16_t airplaneVector;

  uint16_t helicopterSymbol;
  uint16_t helicopterVector;
};

extern Theme kRadarTheme;

}  // namespace ui::radar
