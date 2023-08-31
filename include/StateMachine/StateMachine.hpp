#pragma once

#include <Fatal/Fatal.hpp>
#include <Logger/Logger.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <utility>

struct State {
  State(uint8_t id, const char *name) : id(id), name(name){};
  State(State &&) = delete;
  State(const State &) = delete;
  State &operator=(State &&) = delete;
  State &operator=(const State &) = delete;

  uint8_t id;
  const char *name;
  virtual void execute() = 0;
};

template <size_t numberOfStates, size_t numberOfTransitions>
class StateMachine {
private:
  std::array<std::array<State *, numberOfTransitions>, numberOfStates>
      transitionTable;

  State *currentState;

public:
  StateMachine(State *initial) : currentState(initial) {
    transitionTable.fill(nullptr);
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

    State *nextState = transitionTable[currentState][input];

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
      serial.log(LogLevel::FAILURE, "Transition input out of bounds:", s.name);
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
