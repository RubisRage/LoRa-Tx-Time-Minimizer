#pragma once

#include <array>
#include <cstdint>

/* Possible bandwidths */
extern std::array<double, 10> bandwidth_kHz;

struct LoRaConfig {
  uint8_t bandwidthIndex;
  uint8_t spreadingFactor;
  uint8_t codingRate;
  uint8_t txPower;
};

extern LoRaConfig defaultConfig;
