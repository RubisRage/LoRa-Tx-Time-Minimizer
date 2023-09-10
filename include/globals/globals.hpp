#pragma once

#include "DutyCycleManager/ArduinoClock.hpp"
#include "types/LoRaConfig.hpp"
#include <cstdint>

#ifdef MASTER_BOARD
constexpr uint8_t localAddress = 0xb0;
constexpr uint8_t remoteAddress = 0xb1;
constexpr uint16_t idPrefix = 0x8000;
constexpr float TIMEOUT_SCALE = 2.5;
#endif

#ifdef SLAVE_BOARD
constexpr uint8_t localAddress = 0xb1;
constexpr uint8_t remoteAddress = 0xb0;
constexpr uint16_t idPrefix = 0x0000;
constexpr float TIMEOUT_SCALE = 2.0;
#endif

constexpr auto INITIAL_INTERVAL_BETWEEN_TX = std::chrono::milliseconds(10000);

extern ArduinoClock::duration TIMEOUT;

extern const LoRaConfig defaultConfig;
extern LoRaConfig localNodeConf;
extern LoRaConfig lastNodeConf;
extern LoRaConfig nextNodeConf;
extern uint16_t msgCount;
