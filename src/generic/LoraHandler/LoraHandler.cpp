#include "types/Message.hpp"
#include <LoRa.h>
#include <Logger/Logger.hpp>
#include <LoraHandler/LoraHandler.hpp>
#include <cstring>
#include <globals/globals.hpp>
#include <stdlib.h>

LoraHandler::LoraHandler()
    : lastReceived(), _hasBeenRead(true),
      dutyCycleManager(INITIAL_INTERVAL_BETWEEN_TX) {}

bool LoraHandler::send(Message &message) {
  if (!dutyCycleManager.canTransmit())
    return false;

  dutyCycleManager.beginTx();

  while (!LoRa.beginPacket()) {
    delay(10);
  }

  LoRa.write(message.destinationAddress);
  LoRa.write(message.sourceAddress);
  LoRa.write((uint8_t)(message.id >> 8));
  LoRa.write((uint8_t)(message.id & 0xFF));
  LoRa.write(message.type);
  LoRa.write(message.payloadLength);
  LoRa.write(message.payload, (size_t)message.payloadLength);

  LoRa.endPacket();

  serial.log(LogLevel::INFO, "Sending message:", message);
  dutyCycleManager.updateIntervalBetweenTx();

  /* Enable receiving, which is disabled when transmitting */
  // LoRa.receive();

  return true;
}

inline bool LoraHandler::packetNotEnded(uint8_t receivedBytes, int packetSize) {
  return receivedBytes <= uint8_t(payload.size() - 1) &&
         receivedBytes < lastReceived.payloadLength &&
         Message::headerSize() + receivedBytes < packetSize;
}

void LoraHandler::storeMessage() {

  int packetSize = LoRa.parsePacket();

  if (packetSize == 0)
    return;

  lastReceived.destinationAddress = LoRa.read();
  lastReceived.sourceAddress = LoRa.read();
  lastReceived.id = ((uint16_t)LoRa.read() << 8) | (uint16_t)LoRa.read();
  lastReceived.type = MessageType(LoRa.read());
  lastReceived.payloadLength = LoRa.read();

  lastReceived.payload = payload.data();

  uint8_t receivedBytes = 0;
  while (packetNotEnded(receivedBytes, packetSize)) {
    if (LoRa.available()) {
      payload[receivedBytes++] = (uint8_t)LoRa.read();
    } else {
      serial.log(LogLevel::DEBUG, "Byte not read");
    }
  }

  if (lastReceived.payloadLength != receivedBytes) {
    serial.log(LogLevel::ERROR, "Receiving error: declared message length ",
               lastReceived.payloadLength, " does not match length ",
               receivedBytes);
    lastReceived.payloadLength = 0;

    serial.log(LogLevel::ERROR, "Received message (Payload omitted)",
               lastReceived);

    return;
  }

  if ((lastReceived.destinationAddress & localAddress) != localAddress) {
    serial.log(
        LogLevel::WARNING,
        "Received message was not meant for local node, destination address:",
        lastReceived.destinationAddress, ", dropping.");

    return;
  }

  serial.log(LogLevel::INFO, "Received message:", lastReceived);
  serial.log(LogLevel::INFO, "Packet RSSI:", LoRa.packetRssi(),
             "SNR: ", LoRa.packetSnr());

  _hasBeenRead = false;
}

void LoraHandler::setup(const LoRaConfig &config, void (*onReceive)(int)) {
  LoRa.setSyncWord(0x12);
  LoRa.setPreambleLength(8);
  LoRa.enableCrc();

  updateConfig(config);
}

void LoraHandler::updateConfig(const LoRaConfig &config) {
  LoRa.setSignalBandwidth(long(bandwidth_kHz[config.bandwidthIndex]));
  LoRa.setSpreadingFactor(config.spreadingFactor);
  LoRa.setCodingRate4(config.codingRate);
  LoRa.setTxPower(config.txPower, PA_OUTPUT_PA_BOOST_PIN);
}

bool LoraHandler::canTransmit() { return dutyCycleManager.canTransmit(); }

Message LoraHandler::getMessage() {
  _hasBeenRead = true;
  return lastReceived;
}

bool LoraHandler::hasBeenRead() { return _hasBeenRead; }

/* Global definitions (extern) */
LoraHandler loraHandler;
