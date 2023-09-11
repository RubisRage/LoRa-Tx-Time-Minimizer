#include "states/states_slave.hpp"
#include "LoRa.h"
#include "Logger/Logger.hpp"
#include "Timeout/Timeout.hpp"
#include "states.hpp"
#include "states/states_master.hpp"
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

  if (loraHandler.hasBeenRead()) {
    return;
  }

  Message message = loraHandler.getMessage();

  switch (message.type) {
  case MessageType::ECHO_REQ:
    stateMachine.transition(&SlaveStates::sendEchoReply);
    break;
  case MessageType::CONFIG_START:
    stateMachine.transition(&SlaveStates::waitConfigSet);
    break;
  default:
    serial.log(LogLevel::ERROR, current, "Received",
               messageTypesNames[message.type], "message.");
    break;
  }
}

void sendEchoReply(const State &current) {
  if (!loraHandler.canTransmit()) {
    return;
  }

  delay(500);

  Message echoReply(msgCount++, MessageType::ECHO_REPLY);

  loraHandler.send(echoReply);

  stateMachine.transition(&SlaveStates::listenLoRaPackages);
}

void updateConfig(const State &current) {
  Message configRequest = loraHandler.getMessage();

  LoRaConfig newConf(configRequest.payload);

  serial.log(LogLevel::INFO, "Updating LoRa configuration to:", newConf);
  loraHandler.updateConfig(newConf);
  lastNodeConf = localNodeConf;
  localNodeConf = newConf;

  stateMachine.transition(&SlaveStates::sendConfigAck);
}

void sendConfigAck(const State &current) {
  if (!loraHandler.canTransmit())
    return;

  delay(500);

  Message configSet(msgCount++, MessageType::CONFIG_ACK);

  loraHandler.send(configSet);

  stateMachine.transition(&SlaveStates::waitConfigSet);
}

void fallbackToDefault(const State &current) {
  serial.log(LogLevel::WARNING, "Falling back to default config!");

  if (localNodeConf.bandwidthIndex == defaultConfig.bandwidthIndex &&
      localNodeConf.spreadingFactor == defaultConfig.spreadingFactor) {
    serial.log(LogLevel::WARNING, "Already using default config!");
  } else {
    serial.log(LogLevel::INFO, "Config set:", defaultConfig);

    loraHandler.updateConfig(defaultConfig);
    localNodeConf = defaultConfig;
  }

  stateMachine.transition(&SlaveStates::listenLoRaPackages);
}

void fallbackToPrevious(const State &current) {
  serial.log(LogLevel::WARNING, "Falling back to previous config!");

  if (localNodeConf.bandwidthIndex == lastNodeConf.bandwidthIndex &&
      localNodeConf.spreadingFactor == lastNodeConf.spreadingFactor) {
    serial.log(LogLevel::WARNING, "Local config is already previous config!");
    stateMachine.transition(&SlaveStates::fallbackToDefault);
    return;
  }

  serial.log(LogLevel::INFO, "Config set:", lastNodeConf);
  loraHandler.updateConfig(lastNodeConf);
  localNodeConf = lastNodeConf;

  stateMachine.transition(&SlaveStates::waitConfigSet);
}

void waitConfigSet(const State &current) {
  static Timeout timeout;

  if (loraHandler.hasBeenRead()) {
    if (timeout.hasTimedOut()) {
      serial.log(LogLevel::ERROR, current, "Timed out");
      stateMachine.transition(&SlaveStates::fallbackToPrevious);
    }

    return;
  }

  timeout.reset();

  Message request = loraHandler.getMessage();

  switch (request.type) {
  case MessageType::CONFIG_SET:
    stateMachine.transition(&SlaveStates::updateConfig);
    break;
  case MessageType::CONFIG_END:
    stateMachine.transition(&SlaveStates::listenLoRaPackages);
    break;
  default:

    serial.log(LogLevel::FAILURE, current, "Received non-config message!");
    Fatal::exit();

    break;
  }
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

State SlaveStates::sendConfigAck = {.name = "Send Config ACK",
                                    .action = Actions::sendConfigAck};

State SlaveStates::fallbackToDefault = {.name = "Fallback to default config",
                                        .action = Actions::fallbackToDefault};

State SlaveStates::fallbackToPrevious = {.name = "Fallback to previous config",
                                         .action = Actions::fallbackToPrevious};

State SlaveStates::waitConfigSet = {.name = "Wait on configSet (or configEnd)",
                                    .action = Actions::waitConfigSet};

StateMachine stateMachine(&SlaveStates::listenLoRaPackages);
