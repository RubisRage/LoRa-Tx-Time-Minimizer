#pragma once

#include <StateMachine/StateMachine.hpp>

namespace MasterStates {
extern State testState;
extern State sendEchoRequest;
extern State waitEchoReply;
extern State initialState;
} // namespace MasterStates
