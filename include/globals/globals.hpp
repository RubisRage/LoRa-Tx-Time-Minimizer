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

// constexpr LoRaConfig defaultConfig;
extern LoRaConfig localNodeConf;
