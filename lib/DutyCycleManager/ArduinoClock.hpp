#pragma once

#include <Arduino.h>
#include <chrono>

struct ArduinoClock {
  typedef std::chrono::milliseconds duration;
  typedef duration::rep rep;
  typedef duration::period period;
  typedef std::chrono::time_point<ArduinoClock> time_point;
  static const bool is_steady = true;

  static time_point now() {
    return time_point(ArduinoClock::duration{millis()});
  }
};
