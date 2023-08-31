#include <globals/globals.hpp>

#ifdef MASTER_BOARD
const uint8_t localAddress = 0xb0;
const uint8_t remoteAddress = 0xb1;
#endif

#ifdef SLAVE_BOARD
const uint8_t localAddress = 0xb1;
const uint8_t remoteAddress = 0xb0;
#endif
