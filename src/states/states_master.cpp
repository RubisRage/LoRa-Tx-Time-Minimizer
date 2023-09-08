#include "states/states_master.hpp"
#include "DutyCycleManager/ArduinoClock.hpp"
#include "LoRa.h"
#include "Logger/Logger.hpp"
#include "StateMachine/StateMachine.hpp"
#include "Timeout/Timeout.hpp"
#include "states.hpp"
#include "types/Message.hpp"
#include <LoraHandler/LoraHandler.hpp>
#include <chrono>
#include <globals/globals.hpp>

namespace Actions {

void testAction(const State &current) {

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

    loraHandler.send(message);
    serial.log(LogLevel::INFORMATION, "Sending message: ", message);
  }
}

void sendEchoRequest(const State &current) {

  if (!loraHandler.canTransmit()) {
    return;
  }

  Message echoRequest(msgCount, MessageType::ECHO_REQ);

  loraHandler.send(echoRequest);

  serial.log(LogLevel::INFORMATION, "Sending message:", echoRequest);

  stateMachine.transition(&MasterStates::waitEchoReply);
}

void waitEchoReply(const State &current) {
  static Timeout timeout(TIMEOUT);

  Message echoReply;

  if (!loraHandler.get(echoReply)) {
    if (timeout.hasTimedOut()) {
      serial.log(LogLevel::ERROR, current, "Timed out");
      stateMachine.transition(&MasterStates::initialState);
    }

    return;
  }

  timeout.reset();

  if (echoReply.type != MessageType::ECHO_REPLY) {
    serial.log(LogLevel::ERROR, current, "Unexpected message type received");
  }

  stateMachine.transition(&MasterStates::initialState);
}

void initialState(const State &current) {
  stateMachine.transition(&MasterStates::sendEchoRequest);
}

} // namespace Actions

State MasterStates::testState = {.name = "Test state",
                                 .action = Actions::testAction};

State MasterStates::sendEchoRequest = {.name = "Send echo request",
                                       .action = Actions::sendEchoRequest};

State MasterStates::waitEchoReply = {.name = "Wait echo reply",
                                     .action = Actions::waitEchoReply};

State MasterStates::initialState = {.name = "Initial state",
                                    .action = Actions::initialState};

StateMachine stateMachine(&MasterStates::initialState);
