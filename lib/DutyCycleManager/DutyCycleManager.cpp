#include <DutyCycleManager.hpp>

DutyCycleManager::DutyCycleManager(ArduinoClock::duration initialTxDelay) 
        : lastTxBegin(), txBegin(), txDelay(initialTxDelay) {}

void DutyCycleManager::updateIntervalBetweenTx() {
    ArduinoClock::duration txDuration = ArduinoClock::now() - txBegin;
    ArduinoClock::duration sinceLastMessage = txBegin - lastTxBegin;

    lastTxBegin = txBegin;
    float dutyCycle = 100.0f * txDuration / sinceLastMessage;

    if(dutyCycle > 1.0f) {
        txDelay = txDuration * 100;
    }
}

bool DutyCycleManager::canTransmit() {
    return (ArduinoClock::now() - lastTxBegin) > txDelay;
}
