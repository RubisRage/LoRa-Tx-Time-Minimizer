#include "states/states_master.hpp"
#include "DutyCycleManager/ArduinoClock.hpp"
#include "LoRa.h"
#include "Logger/Logger.hpp"
#include "StateMachine/StateMachine.hpp"
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

  Message echoRequest;

  echoRequest.id = 0;
  echoRequest.type = MessageType::ECHO_REQ;
  echoRequest.sourceAddress = localAddress;
  echoRequest.destinationAddress = remoteAddress;
  echoRequest.payload = 0;
  echoRequest.payloadLength = 0;

  loraHandler.send(echoRequest);

  stateMachine.transition(&MasterStates::waitEchoReply);
}

void waitEchoReply(const State &current) {
  using namespace std::chrono_literals;
  static ArduinoClock::time_point waitStart;
  static const ArduinoClock::duration TIMEOUT = 100000ms;
  static bool waitStarted = false;

  Message echoReply;

  if (!loraHandler.get(echoReply)) {
    ArduinoClock::duration waited = 0ms;

    if (!waitStarted) {
      waitStarted = true;
      waitStart = ArduinoClock::now();
    } else {
      waited = ArduinoClock::now() - waitStart;
    }

    if (waited > TIMEOUT) {
      serial.log(LogLevel::ERROR, current, "Timed out");

      waitStarted = false;
      stateMachine.transition(nullptr); // TODO: Specify corresponding
                                        // transition
    }

    return;
  }

  waitStarted = false;

  if (echoReply.type != MessageType::ECHO_REPLY) {
    serial.log(LogLevel::ERROR, current, "Unexpected message type received");
    return;
  }
}

} // namespace Actions

State MasterStates::testState = {.name = "Test state",
                                 .action = Actions::testAction};

State MasterStates::sendEchoRequest = {.name = "Send echo request",
                                       .action = Actions::sendEchoRequest};

State MasterStates::waitEchoReply = {.name = "Wait echo reply",
                                     .action = Actions::waitEchoReply};

StateMachine stateMachine(&MasterStates::testState);
