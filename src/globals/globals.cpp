#include "types/LoRaConfig.hpp"
#include <chrono>
#include <globals/globals.hpp>

// const LoRaConfig defaultConfig(7, 7, 5, 2);
const LoRaConfig defaultConfig(6, 10, 5, 2);
LoRaConfig localNodeConf = defaultConfig;
LoRaConfig lastNodeConf = defaultConfig;
LoRaConfig nextNodeConf = defaultConfig;
uint16_t msgCount;

using namespace std::chrono;

ArduinoClock::duration TIMEOUT =
    milliseconds(int(INITIAL_INTERVAL_BETWEEN_TX.count() * TIMEOUT_SCALE));
