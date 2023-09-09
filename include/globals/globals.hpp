#pragma once

#include "DutyCycleManager/ArduinoClock.hpp"
#include "types/LoraTypes.hpp"
#include <cstdint>

#ifdef MASTER_BOARD
constexpr uint8_t localAddress = 0xb0;
constexpr uint8_t remoteAddress = 0xb1;
constexpr uint16_t idMask = 0x8000;
#endif

#ifdef SLAVE_BOARD
constexpr uint8_t localAddress = 0xb1;
constexpr uint8_t remoteAddress = 0xb0;
constexpr uint16_t idMask = 0x0000;
#endif

constexpr auto INITIAL_INTERVAL_BETWEEN_TX = std::chrono::milliseconds(10000);
constexpr uint8_t TIMEOUT_SCALE = 3;
extern ArduinoClock::duration TIMEOUT;

extern LoRaConfig localNodeConf;
extern LoRaConfig lastNodeConf;
extern LoRaConfig nextNodeConf;
extern uint16_t msgCount;
