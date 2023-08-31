#pragma once

#ifdef MASTER_BOARD
#include "master/states.hpp"
#endif

#ifdef SLAVE_BOARD
#include "slave/states.hpp"
#endif

#if !(defined(MASTER_BOARD) || defined(SLAVE_BOARD))
#error "MASTER_BOARD or SLAVE_BOARD macros must be defined."
#endif
