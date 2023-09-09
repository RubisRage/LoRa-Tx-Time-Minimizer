#include "states/states_slave.hpp"
#include "LoRa.h"
#include "Logger/Logger.hpp"
#include "states.hpp"
#include "types/LoraTypes.hpp"
#include "types/Message.hpp"
#include <LoraHandler/LoraHandler.hpp>
#include <globals/globals.hpp>

namespace Actions {

void testState(const State &current) {

  if (loraHandler.hasBeenRead())
    return;

  Message message = loraHandler.getMessage();

  if (message.payloadLength != 4) {
    serial.log(LogLevel::ERROR,
               "Unexpected payload size: ", message.payloadLength, "bytes");
    return;
  }

  LoRaConfig remoteNodeConf;
  auto &payload = message.payload;
  int remoteRSSI;
  int remoteSNR;

  remoteNodeConf.bandwidthIndex = payload[0] >> 4;
  remoteNodeConf.spreadingFactor = 6 + ((payload[0] & 0x0F) >> 1);
  remoteNodeConf.codingRate = 5 + (payload[1] >> 6);
  remoteNodeConf.txPower = 2 + ((payload[1] & 0x3F) >> 1);
  remoteRSSI = -int(payload[2]) / 2.0f;
  remoteSNR = int(payload[3]) - 148;

  serial.log(LogLevel::STATISTICS, "Remote node config:", remoteNodeConf);
  serial.log(LogLevel::STATISTICS, "Remote RSSI:", remoteRSSI,
             "dBm, Remote SNR:", remoteSNR, "dB");
}

void listenLoRaPackages(const State &current) {
  if (loraHandler.hasBeenRead())
    return;

  Message message = loraHandler.getMessage();

  serial.log(LogLevel::INFORMATION, "Received message:", message);

  switch (message.type) {
  case MessageType::ECHO_REQ:
    stateMachine.transition(&SlaveStates::sendEchoReply);
    break;
  case MessageType::CONFIG_REQ:
    break;
  case MessageType::FALLBACK_REQ:
    break;
  default:
    serial.log(LogLevel::ERROR, current, "Received non-request message.");
    break;
  }
}

void sendEchoReply(const State &current) {
  if (!loraHandler.canTransmit()) {
    return;
  }

  Message echoReply(msgCount++, MessageType::ECHO_REPLY);

  loraHandler.send(echoReply);

  serial.log(LogLevel::INFORMATION, "Sending echoReply: ", echoReply);

  stateMachine.transition(&SlaveStates::listenLoRaPackages);
}

} // namespace Actions

State SlaveStates::testState = {.name = "Test state",
                                .action = Actions::testState};

State SlaveStates::listenLoRaPackages = {.name = "Listen LoRa packages",
                                         .action = Actions::listenLoRaPackages};

State SlaveStates::sendEchoReply = {.name = "Send Echo Reply",
                                    .action = Actions::sendEchoReply};

StateMachine stateMachine(&SlaveStates::listenLoRaPackages);
