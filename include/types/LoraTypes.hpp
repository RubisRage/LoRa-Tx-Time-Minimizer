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
// extern std::array<double, 10> bandwidth_kHz;
constexpr std::array<double, 10> bandwidth_kHz = {
    7.8E3,  10.4E3, 15.6E3, 20.8E3, 31.25E3,
    41.7E3, 62.5E3, 125E3,  250E3,  500E3};
constexpr LoRaConfig defaultConfig = {
    .bandwidthIndex = 6, .spreadingFactor = 10, .codingRate = 5, .txPower = 2};
