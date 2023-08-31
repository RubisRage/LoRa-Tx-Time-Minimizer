#include <LoRa.h>
#include <Logger/Logger.hpp>
#include <LoraHandler/LoraHandler.hpp>
#include <globals/globals.hpp>

LoraHandler::LoraHandler()
    : lastReceived(), validMessage(false),
      dutyCycleManager(INITIAL_INTERVAL_BETWEEN_TX), transmitting(false),
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
  LoRa.setSyncWord(0x12);
  LoRa.setPreambleLength(8);
  LoRa.onReceive([](int packetSize) { loraHandler.onReceive(packetSize); });
  LoRa.onTxDone([] { loraHandler.txDone = true; });

  updateConfig(config);
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

void LoraHandler::onReceive(int packetSize) {
  if (packetSize == 0) {
    serial.log(LogLevel::WARNING, "\"Received\" empty packet.");
    return;
  }

  Message message;

  message.destinationAddress = LoRa.read();
  message.sourceAddress = LoRa.read();
  message.id = ((uint16_t)LoRa.read() << 7) | (uint16_t)LoRa.read();
  message.type = MessageType(LoRa.read());
  message.payloadLength = LoRa.read();

  std::array<uint8_t, 10> payload;
  message.payload = payload.data();

  uint8_t receivedBytes = 0;
  while (LoRa.available() && (receivedBytes < uint8_t(payload.size() - 1)) &&
         receivedBytes < message.payloadLength) {
    payload[receivedBytes++] = (char)LoRa.read();
  }

  while (LoRa.available())
    LoRa.read();

  if (message.payloadLength != receivedBytes) {
    serial.log(LogLevel::ERROR, "Receiving error: declared message length ",
               message.payloadLength, " does not match length ", receivedBytes);
    return;
  }

  if ((message.destinationAddress & localAddress) != localAddress) {
    serial.log(LogLevel::WARNING,
               "Received message was not meant for localhost, dropping.");
    return;
  }

  serial.log(LogLevel::INFORMATION, "Received message", message);

  serial.log(LogLevel::INFORMATION, "Local RSSI:", LoRa.packetRssi(),
             "dBm, Local SNR:", LoRa.packetSnr(), "dB");

  lastReceived = message;
  validMessage = true;
}

bool LoraHandler::get(Message &message) {
  message = lastReceived;
  validMessage = false;
}

/* Global definitions (extern) */
LoraHandler loraHandler;
std::array<double, 10> bandwidth_kHz = {7.8E3,  10.4E3, 15.6E3, 20.8E3, 31.25E3,
                                        41.7E3, 62.5E3, 125E3,  250E3,  500E3};
LoRaConfig defaultConfig = {
    .bandwidthIndex = 6, .spreadingFactor = 10, .codingRate = 5, .txPower = 2};
