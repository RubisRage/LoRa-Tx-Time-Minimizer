#pragma once

#include "DutyCycleManager/ArduinoClock.hpp"

class Timeout {
public:
  Timeout();
  ~Timeout() = default;
  Timeout(Timeout &&) = delete;
  Timeout(const Timeout &) = delete;
  Timeout &operator=(Timeout &&) = delete;
  Timeout &operator=(const Timeout &) = delete;

  void reset();
  bool hasTimedOut();

private:
  bool waitStarted;
  ArduinoClock::time_point waitStart;
};
