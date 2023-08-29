#include <DutyCycleManager.hpp>

DutyCycleManager::DutyCycleManager(ArduinoClock::duration initialTxDelay)
    : lastTxBegin(), txBegin(), txDelay(initialTxDelay), dutyCycle() {}

void DutyCycleManager::updateIntervalBetweenTx() {
  ArduinoClock::duration txDuration = ArduinoClock::now() - txBegin;
  ArduinoClock::duration sinceLastMessage = txBegin - lastTxBegin;

  lastTxBegin = txBegin;
  dutyCycle = 100.0f * txDuration / sinceLastMessage;

  Serial.print("----> TX completed in ");
  Serial.print(txDuration.count());
  Serial.println(" msecs");

  if (dutyCycle > 1.0f) {
    txDelay = txDuration * 100;
  }
}

bool DutyCycleManager::canTransmit() {
  return (ArduinoClock::now() - lastTxBegin) > txDelay;
}

void DutyCycleManager::beginTx() { txBegin = ArduinoClock::now(); }
