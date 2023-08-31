#include <DutyCycleManager/DutyCycleManager.hpp>
#include <Logger/Logger.hpp>

DutyCycleManager::DutyCycleManager(ArduinoClock::duration initialTxDelay)
    : lastTxBegin(), txBegin(), txDelay(initialTxDelay), dutyCycle() {}

void DutyCycleManager::updateIntervalBetweenTx() {
  ArduinoClock::duration txDuration = ArduinoClock::now() - txBegin;
  ArduinoClock::duration sinceLastMessage = txBegin - lastTxBegin;

  lastTxBegin = txBegin;
  dutyCycle = 100.0f * txDuration / sinceLastMessage;

  serial.log(LogLevel::STATISTICS, "Tx completed in", (int)txDuration.count(),
             "ms");

  serial.log(LogLevel::STATISTICS, "Duty cycle is", dutyCycle, "%");

  if (dutyCycle > 1.0f) {
    serial.log(LogLevel::WARNING, "Duty cycle exceeded 1%, adjusting.");
    txDelay = txDuration * 100;
  }
}

bool DutyCycleManager::canTransmit() {
  return (ArduinoClock::now() - lastTxBegin) > txDelay;
}

void DutyCycleManager::beginTx() { txBegin = ArduinoClock::now(); }
