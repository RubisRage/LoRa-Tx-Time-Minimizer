#pragma once

#include "Logger/Logger.hpp"
#include <Fatal/Fatal.hpp>
#include <types/State.hpp>

class StateMachine {
private:
  const State *currentState;

public:
  StateMachine(const State *initial) : currentState(initial) {
    serial.log(LogLevel::INFORMATION,
               "State machine initialized with starting state",
               currentState->name);
  }

  ~StateMachine() = default;
  StateMachine &operator=(StateMachine &&) = delete;
  StateMachine(StateMachine &&) = delete;
  StateMachine(const StateMachine &) = delete;
  StateMachine &operator=(const StateMachine &) = delete;

  void transition(const State *newState) {
    if (newState == nullptr) {
      serial.log(LogLevel::FAILURE, "Tried transitioning to nullptr");
      Fatal::exit();
    }

    serial.log(LogLevel::TRANSITION, "Transitioning from \"",
               currentState->name, "\" to \"", newState->name, "\"");

    currentState = newState;
  };

  const State &getState() { return *currentState; }
};
