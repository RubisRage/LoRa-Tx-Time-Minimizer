#pragma once

#include "StateMachine/StateMachine.hpp"
#include <cstdint>

struct TestState : State {
  TestState(uint8_t id, const char *name) : State(id, name) {}
  void execute() override;
};
