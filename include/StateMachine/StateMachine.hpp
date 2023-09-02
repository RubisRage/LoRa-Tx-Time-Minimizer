#pragma once

#include <Fatal/Fatal.hpp>
#include <Logger/Logger.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <utility>

struct State {
  uint8_t id;
  const char *name;
  void (*execute)();
};

template <size_t numberOfStates, size_t numberOfTransitions>
class StateMachine {
private:
  std::array<std::array<const State *, numberOfTransitions>, numberOfStates>
      transitionTable;

  const State *currentState;

public:
  StateMachine(const State *initial) : currentState(initial) {
    for (auto &stateTransitions : transitionTable)
      stateTransitions.fill(nullptr);
  }

  ~StateMachine() = default;
  StateMachine &operator=(StateMachine &&) = delete;
  StateMachine(StateMachine &&) = delete;
  StateMachine(const StateMachine &) = delete;
  StateMachine &operator=(const StateMachine &) = delete;

  void transition(size_t input) {
    if (input > numberOfTransitions) {
      serial.log(LogLevel::FAILURE, "Transition input out of bounds:", input);
      Fatal::exit();
    }

    const State *nextState = transitionTable[currentState][input];

    if (nextState == nullptr) {
      serial.log(LogLevel::FAILURE, "Transitioned to dump state", input);
      Fatal::exit();
    }

    currentState = nextState;
  };

  void registerTransitions(
      const State &s,
      const std::initializer_list<std::pair<size_t, State *>> &transitions) {
    if (s.id > numberOfStates) {
      serial.log(LogLevel::FAILURE, "Registered state out of bounds:", s.name);
      Fatal::exit();
    }

    for (const auto &t : transitions) {
      const size_t input = t.first;
      const State *state = t.second;

      if (input > numberOfTransitions) {
        serial.log(LogLevel::FAILURE, "Transition input out of bounds:", input);
        Fatal::exit();
      }

      transitionTable[s.id][input] = state;
    }
  }

  const State &getState() { return *currentState; }
};
