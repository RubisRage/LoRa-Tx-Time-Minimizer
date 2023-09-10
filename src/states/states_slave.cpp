#include "states/states_slave.hpp"
#include "LoRa.h"
#include "Logger/Logger.hpp"
#include "Timeout/Timeout.hpp"
#include "states.hpp"
#include "types/LoRaConfig.hpp"
#include "types/Message.hpp"
#include "types/State.hpp"
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
  static Timeout timeout;

  if (loraHandler.hasBeenRead()) {
    if (timeout.hasTimedOut()) {
      serial.log(LogLevel::ERROR, current, "Timed out");
      stateMachine.transition(&SlaveStates::fallbackToDefault);
    }

    return;
  }

  timeout.reset();

  Message message = loraHandler.getMessage();

  serial.log(LogLevel::INFO, "Received message:", message);

  switch (message.type) {
  case MessageType::ECHO_REQ:
    stateMachine.transition(&SlaveStates::sendEchoReply);
    break;
  case MessageType::CONFIG_REQ:
    stateMachine.transition(&SlaveStates::updateConfig);
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

  serial.log(LogLevel::INFO, "Sending echoReply: ", echoReply);

  stateMachine.transition(&SlaveStates::listenLoRaPackages);
}

void updateConfig(const State &current) {
  Message configRequest = loraHandler.getMessage();

  LoRaConfig newConf(configRequest.payload);

  serial.log(LogLevel::INFO, "Updating LoRa configuration to:", newConf);
  loraHandler.updateConfig(newConf);
  lastNodeConf = localNodeConf;
  localNodeConf = newConf;

  stateMachine.transition(&SlaveStates::sendConfigSet);
}

void sendConfigSet(const State &current) {
  if (!loraHandler.canTransmit())
    return;

  Message configSet(msgCount++, MessageType::CONFIG_SET);

  loraHandler.send(configSet);

  serial.log(LogLevel::INFO, "Sending configSet:", configSet);

  stateMachine.transition(&SlaveStates::listenLoRaPackages);
}

void fallbackToDefault(const State &current) {
  serial.log(LogLevel::WARNING, "Falling back to default config!");

  if (localNodeConf.bandwidthIndex == defaultConfig.bandwidthIndex &&
      localNodeConf.spreadingFactor == defaultConfig.spreadingFactor) {
    serial.log(LogLevel::FAILURE, "Already using default config!");
    Fatal::exit();
  }

  loraHandler.updateConfig(defaultConfig);
  localNodeConf = defaultConfig;

  stateMachine.transition(&SlaveStates::listenLoRaPackages);
}

} // namespace Actions

State SlaveStates::testState = {.name = "Test state",
                                .action = Actions::testState};

State SlaveStates::listenLoRaPackages = {.name = "Listen LoRa packages",
                                         .action = Actions::listenLoRaPackages};

State SlaveStates::sendEchoReply = {.name = "Send Echo Reply",
                                    .action = Actions::sendEchoReply};

State SlaveStates::updateConfig = {.name = "Update config",
                                   .action = Actions::updateConfig};

State SlaveStates::sendConfigSet = {.name = "Send Config Set",
                                    .action = Actions::sendConfigSet};

State SlaveStates::fallbackToDefault = {.name = "Fallback to default config",
                                        .action = Actions::fallbackToDefault};

StateMachine stateMachine(&SlaveStates::listenLoRaPackages);
