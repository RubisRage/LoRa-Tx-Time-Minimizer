#pragma once

#include <StateMachine/StateMachine.hpp>

namespace SlaveStates {

extern State testState;
extern State listenLoRaPackages;
extern State sendEchoReply;
extern State updateConfig;
extern State sendConfigSet;
extern State fallbackToDefault;

} // namespace SlaveStates
