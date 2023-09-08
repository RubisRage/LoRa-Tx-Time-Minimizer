#pragma once

#include "DutyCycleManager/ArduinoClock.hpp"

class Timeout {
public:
  Timeout(const ArduinoClock::duration TIMEOUT);
  ~Timeout() = default;
  Timeout(Timeout &&) = delete;
  Timeout(const Timeout &) = delete;
  Timeout &operator=(Timeout &&) = delete;
  Timeout &operator=(const Timeout &) = delete;

  // private:

  void reset();
  bool hasTimedOut();

  bool waitStarted;
  ArduinoClock::time_point waitStart;
  const ArduinoClock::duration TIMEOUT;
};
