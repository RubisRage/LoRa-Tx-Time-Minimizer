#include <LoRa.h>
#include <LoraHandler/LoraHandler.hpp>
#include <functional>

LoraHandler::LoraHandler()
    : dutyCycleManager(INITIAL_INTERVAL_BETWEEN_TX), transmitting(false),
      txDone(true) {}

bool LoraHandler::sendMessage(Message message) {
  if (!dutyCycleManager.canTransmit())
    return false;

  transmitting = true;
  txDone = false;
  dutyCycleManager.beginTx();

  while (!LoRa.beginPacket()) {
    delay(10);
  }

  LoRa.write(message.destinationAddress);
  LoRa.write(message.sourceAddress);
  LoRa.write((uint8_t)(message.id >> 7));
  LoRa.write((uint8_t)(message.id & 0xFF));
  LoRa.write(message.type);
  LoRa.write(message.payloadLength);
  LoRa.write(message.payload, (size_t)message.payloadLength);
  LoRa.endPacket(true);

  return true;
}

void LoraHandler::setup(LoRaConfig &config, void (*onReceive)(int)) {
  LoRa.setSignalBandwidth(long(bandwidth_kHz[config.bandwidthIndex]));
  LoRa.setSpreadingFactor(config.spreadingFactor);
  LoRa.setCodingRate4(config.codingRate);
  LoRa.setTxPower(config.txPower, PA_OUTPUT_PA_BOOST_PIN);
  LoRa.setSyncWord(0x12);
  LoRa.setPreambleLength(8);
  LoRa.onReceive(onReceive);
  LoRa.onTxDone([] {
    loraHandler.txDone = true;
    digitalWrite(LED_BUILTIN, HIGH);
  });

  LoRa.receive();
}

void LoraHandler::updateConfig(LoRaConfig &config) {
  LoRa.setSignalBandwidth(long(bandwidth_kHz[config.bandwidthIndex]));
  LoRa.setSpreadingFactor(config.spreadingFactor);
  LoRa.setCodingRate4(config.codingRate);
  LoRa.setTxPower(config.txPower, PA_OUTPUT_PA_BOOST_PIN);

  LoRa.receive();
}

void LoraHandler::updateTransmissionState() {

  if (transmitting && txDone) {

    transmitting = false;

    dutyCycleManager.updateIntervalBetweenTx();

    /* Enable receiving, which is disabled when transmitting */
    LoRa.receive();
  }
}

bool LoraHandler::canTransmit() {
  return !transmitting && dutyCycleManager.canTransmit();
}

/* Global definitions (extern) */
LoraHandler loraHandler;
std::array<double, 10> bandwidth_kHz = {7.8E3,  10.4E3, 15.6E3, 20.8E3, 31.25E3,
                                        41.7E3, 62.5E3, 125E3,  250E3,  500E3};
LoRaConfig defaultConfig = {
    .bandwidthIndex = 6, .spreadingFactor = 10, .codingRate = 5, .txPower = 2};
