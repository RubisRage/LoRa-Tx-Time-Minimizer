#pragma once

#include <array>
#include <cstdint>

/* Possible bandwidths */
constexpr std::array<double, 10> bandwidth_kHz = {
    7.8E3,  10.4E3, 15.6E3, 20.8E3, 31.25E3,
    41.7E3, 62.5E3, 125E3,  250E3,  500E3};

struct LoRaConfig {
  uint8_t bandwidthIndex;
  uint8_t spreadingFactor;
  uint8_t codingRate;
  uint8_t txPower;

  void serialize(std::array<uint8_t, 2> &buffer) const {
    size_t payloadLength = 0;

    buffer[payloadLength] = (bandwidthIndex << 4);
    buffer[payloadLength++] |= ((spreadingFactor - 6) << 1);
    buffer[payloadLength] = ((codingRate - 5) << 6);
    buffer[payloadLength++] |= ((txPower - 2) << 1);
  }

  LoRaConfig() = default;
  LoRaConfig(uint8_t bwi, uint8_t sf, uint8_t cr, uint8_t txp)
      : bandwidthIndex(bwi), spreadingFactor(sf), codingRate(cr), txPower(txp) {
  }

  LoRaConfig(uint8_t buffer[2]);
};
