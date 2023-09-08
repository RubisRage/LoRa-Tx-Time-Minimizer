#include "DutyCycleManager/ArduinoClock.hpp"
#include "types/LoraTypes.hpp"
#include <globals/globals.hpp>

LoRaConfig localNodeConf = defaultConfig;
LoRaConfig lastNodeConf = defaultConfig;
LoRaConfig nextNodeConf = defaultConfig;
uint16_t msgCount;
