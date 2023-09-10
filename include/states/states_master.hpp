#pragma once

#include <StateMachine/StateMachine.hpp>

namespace MasterStates {
extern State testState;
extern State sendEchoRequest;
extern State waitEchoReply;
extern State initialState;
extern State computeNextConfig;
extern State sendConfigRequest;
extern State waitConfigSet;
extern State fallbackToPrevious;
extern State waitFallbackConfigSet;
extern State fallbackOnTimeout;
} // namespace MasterStates
