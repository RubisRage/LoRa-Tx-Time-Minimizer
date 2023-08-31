#pragma once

#include <array>
#include <cstdint>

struct LoRaConfig {
  uint8_t bandwidthIndex;
  uint8_t spreadingFactor;
  uint8_t codingRate;
  uint8_t txPower;
};

/* Possible bandwidths */
extern std::array<double, 10> bandwidth_kHz;
extern LoRaConfig defaultConfig;
