#include "types/LoraTypes.hpp"
#include <chrono>
#include <globals/globals.hpp>

LoRaConfig localNodeConf = defaultConfig;
LoRaConfig lastNodeConf = defaultConfig;
LoRaConfig nextNodeConf = defaultConfig;
uint16_t msgCount;

using namespace std::chrono_literals;
ArduinoClock::duration TIMEOUT = INITIAL_INTERVAL_BETWEEN_TX * TIMEOUT_SCALE;
