#pragma once

#include "DutyCycleManager/ArduinoClock.hpp"
#include "types/LoraTypes.hpp"
#include <cstdint>

#ifdef MASTER_BOARD
constexpr uint8_t localAddress = 0xb0;
constexpr uint8_t remoteAddress = 0xb1;
#endif

#ifdef SLAVE_BOARD
constexpr uint8_t localAddress = 0xb1;
constexpr uint8_t remoteAddress = 0xb0;
#endif

constexpr auto TIMEOUT = ArduinoClock::duration(100000);

extern LoRaConfig localNodeConf;
extern LoRaConfig lastNodeConf;
extern LoRaConfig nextNodeConf;
extern uint16_t msgCount;
