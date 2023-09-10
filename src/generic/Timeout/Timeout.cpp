#include "DutyCycleManager/ArduinoClock.hpp"
#include <Timeout/Timeout.hpp>
#include <globals/globals.hpp>

Timeout::Timeout() : waitStarted(false) {}

bool Timeout::hasTimedOut() {
  if (!waitStarted) {
    waitStarted = true;
    waitStart = ArduinoClock::now();

    return false;
  }

  bool timedout = ArduinoClock::now() - waitStart > TIMEOUT;

  if (timedout) {
    reset();
  }

  return timedout;
}

void Timeout::reset() { waitStarted = false; }
