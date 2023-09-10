#pragma once

#include <StateMachine/StateMachine.hpp>

namespace MasterStates {
extern State testState;
extern State sendEchoRequest;
extern State waitEchoReply;
extern State initialState;
extern State computeNextConfig;
extern State sendConfigRequest;
extern State sendConfigStart;
extern State sendConfigEnd;
extern State waitConfigAck;
extern State fallbackToPrevious;
extern State waitFallbackConfigAck;
extern State fallbackOnTimeout;
} // namespace MasterStates
