#include "states/states_master.hpp"
#include "LoRa.h"
#include "StateMachine/StateMachine.hpp"
#include "states.hpp"
#include <LoraHandler/LoraHandler.hpp>
#include <globals/globals.hpp>

void testStateFunc() {

  static uint16_t msgCount = 0;

  if (loraHandler.canTransmit()) {

    size_t payloadLength = 0;

    std::array<uint8_t, 50> payload;

    payload[payloadLength] = (localNodeConf.bandwidthIndex << 4);
    payload[payloadLength++] |= ((localNodeConf.spreadingFactor - 6) << 1);
    payload[payloadLength] = ((localNodeConf.codingRate - 5) << 6);
    payload[payloadLength++] |= ((localNodeConf.txPower - 2) << 1);
    payload[payloadLength++] = uint8_t(-LoRa.packetRssi() * 2);
    payload[payloadLength++] = uint8_t(148 + LoRa.packetSnr());

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

constexpr State states[] = {
    {.id = 0, .name = "Test state", .execute = testStateFunc}};

StateMachine<1, 0> masterStateMachine(&states[0]);

StateMachine<1, 0> &initializeMasterStateMachine() {
  return masterStateMachine;
}
