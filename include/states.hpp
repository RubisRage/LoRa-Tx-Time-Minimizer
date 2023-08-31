#pragma once

#ifdef MASTER_BOARD
#include <states/states_master.hpp>
#endif

#ifdef SLAVE_BOARD
#include "states/states_slave.hpp"
#endif

#if !(defined(MASTER_BOARD) || defined(SLAVE_BOARD))
#error "MASTER_BOARD or SLAVE_BOARD macros must be defined."
#endif
