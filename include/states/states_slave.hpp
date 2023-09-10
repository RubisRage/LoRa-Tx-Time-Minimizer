#pragma once

#include <StateMachine/StateMachine.hpp>

namespace SlaveStates {

extern State testState;
extern State listenLoRaPackages;
extern State sendEchoReply;
extern State updateConfig;
extern State waitConfigSet;
extern State sendConfigAck;
extern State fallbackToDefault;
extern State fallbackToPrevious;

} // namespace SlaveStates
