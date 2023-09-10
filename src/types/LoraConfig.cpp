#include "Fatal/Fatal.hpp"
#include <Logger/Logger.hpp>
#include <types/LoRaConfig.hpp>

LoRaConfig::LoRaConfig(uint8_t buffer[2]) {
  bandwidthIndex = buffer[0] >> 4;
  spreadingFactor = 6 + ((buffer[0] & 0x0F) >> 1);
  codingRate = 5 + (buffer[1] >> 6);
  txPower = 2 + ((buffer[1] & 0x3F) >> 1);

  if (bandwidthIndex < 0 || bandwidthIndex >= bandwidth_kHz.size() ||
      spreadingFactor > 12 || spreadingFactor < 7 || codingRate < 5 ||
      codingRate > 8 || txPower < 2 || txPower > 20) {
    serial.log(LogLevel::FAILURE,
               "Attempt to create LoRa config with parameters out of range");

    serial.log(LogLevel::FAILURE, "Constructed config:", *this);
    Fatal::exit();
  }
}
