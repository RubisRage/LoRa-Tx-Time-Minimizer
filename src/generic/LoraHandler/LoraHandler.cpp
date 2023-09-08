#include <LoRa.h>
#include <Logger/Logger.hpp>
#include <LoraHandler/LoraHandler.hpp>
#include <globals/globals.hpp>

LoraHandler::LoraHandler()
    : lastReceived(), validMessage(false),
      dutyCycleManager(INITIAL_INTERVAL_BETWEEN_TX), transmitting(false),
      txDone(true) {}

bool LoraHandler::send(Message message) {
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
  LoRa.endPacket();

  dutyCycleManager.updateIntervalBetweenTx();

  /* Enable receiving, which is disabled when transmitting */
  LoRa.receive();

  return true;
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

  message.payload = loraHandler.payload.data();

  uint8_t receivedBytes = 0;
  while (LoRa.available() &&
         (receivedBytes < uint8_t(loraHandler.payload.size() - 1)) &&
         receivedBytes < message.payloadLength) {
    loraHandler.payload[receivedBytes++] = (uint8_t)LoRa.read();
  }

  while (LoRa.available())
    LoRa.read();

  if (message.payloadLength != receivedBytes) {
    serial.log(LogLevel::ERROR, "Receiving error: declared message length ",
               message.payloadLength, " does not match length ", receivedBytes);
    return;
  }

  if ((message.destinationAddress & localAddress) != localAddress) {
    serial.log(
        LogLevel::WARNING,
        "Received message was not meant for local node, destination address:",
        message.destinationAddress, ", dropping.");

    serial.log(LogLevel::DEBUG, "Local address: ", localAddress);
    return;
  }

  serial.log(LogLevel::INFORMATION, "Received message", message);

  serial.log(LogLevel::INFORMATION, "Local RSSI:", LoRa.packetRssi(),
             "dBm, Local SNR:", LoRa.packetSnr(), "dB");

  loraHandler.lastReceived = message;
  loraHandler.validMessage = true;
}

void LoraHandler::setup(const LoRaConfig &config, void (*onReceive)(int)) {
  LoRa.setSyncWord(0x12);
  LoRa.setPreambleLength(8);
  LoRa.onReceive(LoraHandler::onReceive);
  LoRa.onTxDone([] { loraHandler.txDone = true; });

  updateConfig(config);
}

void LoraHandler::updateConfig(const LoRaConfig &config) {
  LoRa.setSignalBandwidth(long(bandwidth_kHz[config.bandwidthIndex]));
  LoRa.setSpreadingFactor(config.spreadingFactor);
  LoRa.setCodingRate4(config.codingRate);
  LoRa.setTxPower(config.txPower, PA_OUTPUT_PA_BOOST_PIN);

  LoRa.receive();
}

void LoraHandler::updateTransmissionState() {}

bool LoraHandler::canTransmit() { return dutyCycleManager.canTransmit(); }

bool LoraHandler::get(Message &message) {
  message = lastReceived;
  bool ret = validMessage;
  validMessage = false;

  return ret;
}

/* Global definitions (extern) */
LoraHandler loraHandler;
