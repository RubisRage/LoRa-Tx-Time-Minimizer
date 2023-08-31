#include "states.hpp"
#include "LoRa.h"
#include <LoraHandler/LoraHandler.hpp>

const uint8_t localAddress = 0xB0;
static uint8_t remoteAddress = 0xFF;

LoRaConfig localNodeConf = defaultConfig;

template <size_t size>
void buildPayload(std::array<uint8_t, size> &payload, size_t &payloadLength) {
#ifdef DEBUG_LOG
  serial.log(LogLevel::DEBUG, "Start build:", payloadLength);
#endif // DEBUG

  payload[payloadLength] = (localNodeConf.bandwidthIndex << 4);
  payload[payloadLength++] |= ((localNodeConf.spreadingFactor - 6) << 1);
  payload[payloadLength] = ((localNodeConf.codingRate - 5) << 6);
  payload[payloadLength++] |= ((localNodeConf.txPower - 2) << 1);
  payload[payloadLength++] = uint8_t(-LoRa.packetRssi() * 2);
  payload[payloadLength++] = uint8_t(148 + LoRa.packetSnr());

#ifdef DEBUG_LOG
  serial.log(LogLevel::DEBUG, "End build:", payloadLength.c_str());
#endif // DEBUG
}

void TestState::execute() {

  static uint16_t msgCount = 0;

  if (loraHandler.canTransmit()) {

    size_t payloadLength = 0;

    std::array<uint8_t, 50> payload;

    buildPayload(payload, payloadLength);

    Message message;
    message.id = msgCount++;
    message.type = MessageType::STATUS;
    message.sourceAddress = localAddress;
    message.destinationAddress = remoteAddress;
    message.payload = payload.data();
    message.payloadLength = payloadLength;

    loraHandler.sendMessage(message);
    serial.log(LogLevel::INFORMATION, "Sending message: ", message);
  }

  loraHandler.updateTransmissionState();
}
