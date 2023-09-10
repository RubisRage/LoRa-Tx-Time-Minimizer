#include "states/states_master.hpp"
#include "DutyCycleManager/ArduinoClock.hpp"
#include "LoRa.h"
#include "Logger/Logger.hpp"
#include "StateMachine/StateMachine.hpp"
#include "Timeout/Timeout.hpp"
#include "states.hpp"
#include "types/LoRaConfig.hpp"
#include "types/Message.hpp"
#include <LoraHandler/LoraHandler.hpp>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <globals/globals.hpp>
#include <iterator>

namespace Actions {

bool hasFalledBack = false;

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
    message.type = MessageType::ECHO_REQ;
    message.sourceAddress = localAddress;
    message.destinationAddress = remoteAddress;
    message.payload = payload.data();
    message.payloadLength = payloadLength;

    loraHandler.send(message);
    serial.log(LogLevel::INFO, "Sending message: ", message);
  }
}

void sendEchoRequest(const State &current) {

  if (!loraHandler.canTransmit()) {
    return;
  }

  Message echoRequest(msgCount++, MessageType::ECHO_REQ);

  loraHandler.send(echoRequest);

  stateMachine.transition(&MasterStates::waitEchoReply);
}

void waitEchoReply(const State &current) {
  static Timeout timeout;

  if (loraHandler.hasBeenRead()) {
    if (timeout.hasTimedOut()) {
      serial.log(LogLevel::ERROR, current, "Timed out");
      stateMachine.transition(&MasterStates::initialState);
    }

    return;
  }

  Message echoReply = loraHandler.getMessage();

  timeout.reset();

  if (echoReply.type != MessageType::ECHO_REPLY) {
    serial.log(LogLevel::ERROR, current, "Unexpected message type received");
  }

  stateMachine.transition(&MasterStates::initialState);
}

void computeNextConfig(const State &current) {

  if (hasFalledBack) {
    hasFalledBack = false;

    serial.log(
        LogLevel::INFO, current,
        "Failed to connect using last upgrade, ending minimization process.");

    stateMachine.transition(&MasterStates::sendConfigEnd);
  }

  struct LoRaThresholds {
    int rssi;
    float snr;
  };

  // TODO: Make "good enough" range tighter
  constexpr LoRaThresholds upper = {-115, -7.0};
  constexpr LoRaThresholds lower = {-120, -13.0};

  int RSSI = LoRa.packetRssi();
  float SNR = LoRa.packetSnr();

  serial.log(LogLevel::INFO,
             "Values used for configuration update: { RSSI:", RSSI,
             ", SNR: ", SNR, "}");

  nextNodeConf = localNodeConf;

  if (RSSI > upper.rssi && SNR > upper.snr) {
    serial.log(LogLevel::INFO, current,
               "RSSI and SNR above threshold, improving transmission time.");

    if (nextNodeConf.spreadingFactor > 7) {
      nextNodeConf.spreadingFactor--;
      stateMachine.transition(&MasterStates::sendConfigRequest);
    } else if (nextNodeConf.bandwidthIndex < bandwidth_kHz.size() - 1) {
      nextNodeConf.bandwidthIndex++;
      stateMachine.transition(&MasterStates::sendConfigRequest);
    } else {
      serial.log(LogLevel::INFO, current,
                 "Optimal configuration has been reached! Transmission time "
                 "minimization has ended.");
      stateMachine.transition(&MasterStates::sendConfigEnd);
    }

  } else if (RSSI <= lower.rssi || SNR <= lower.rssi) {
    serial.log(LogLevel::INFO, current,
               "RSSI and/or SNR below threshold. Falling back to previous "
               "configuration.");
    stateMachine.transition(&MasterStates::fallbackToPrevious);
  } else {
    serial.log(LogLevel::INFO, current,
               "RSSI and SNR within good connection range (Not excellent), "
               "transmission time minimization has ended.");
    stateMachine.transition(&MasterStates::initialState);
  }
}

void initialState(const State &current) {
  static bool promptDisplayed = false;

  if (!promptDisplayed) {
    Serial.print("Introduce command > ");
    promptDisplayed = true;
  }

  constexpr size_t BUF_SIZE = 10;
  char buffer[BUF_SIZE];

  size_t readBytes = Serial.readBytesUntil('\n', buffer, BUF_SIZE - 1);
  buffer[readBytes] = 0;

  if (readBytes != 0)
    promptDisplayed = false;

  if (strcmp(buffer, "echo") == 0) {
    stateMachine.transition(&MasterStates::sendEchoRequest);
  } else if (strcmp(buffer, "minimize") == 0) {
    stateMachine.transition(&MasterStates::sendConfigStart);
  } else if (readBytes != 0) {
    Serial.println("Valid commands: \"echo\" and \"minimize\"");
  }
}

void sendConfigRequest(const State &current) {
  if (!loraHandler.canTransmit())
    return;

  std::array<uint8_t, 2> payload;
  nextNodeConf.serialize(payload);

  Message configRequest(msgCount++, MessageType::CONFIG_SET);
  configRequest.payload = payload.data();
  configRequest.payloadLength = payload.size();

  loraHandler.send(configRequest);

  serial.log(LogLevel::INFO, "Updating LoRa configuration to:", nextNodeConf);
  loraHandler.updateConfig(nextNodeConf);
  lastNodeConf = localNodeConf;
  localNodeConf = nextNodeConf;

  stateMachine.transition(&MasterStates::waitConfigAck);
}

void sendConfigStart(const State &current) {
  if (!loraHandler.canTransmit())
    return;

  Message configStart(msgCount++, MessageType::CONFIG_START);

  loraHandler.send(configStart);

  stateMachine.transition(&MasterStates::computeNextConfig);
}

void sendConfigEnd(const State &current) {
  if (!loraHandler.canTransmit())
    return;

  Message configEnd(msgCount++, MessageType::CONFIG_END);

  loraHandler.send(configEnd);

  stateMachine.transition(&MasterStates::initialState);
}

void waitConfigAck(const State &current) {
  static Timeout timeout;

  if (loraHandler.hasBeenRead()) {
    if (timeout.hasTimedOut()) {
      serial.log(LogLevel::ERROR, current, "Timed out");
      stateMachine.transition(&MasterStates::fallbackToPrevious);
    }

    return;
  }

  timeout.reset();

  Message configSet = loraHandler.getMessage();

  if (configSet.type != MessageType::CONFIG_ACK) {
    serial.log(LogLevel::FAILURE, current, "Received wrong message type!");
    serial.log(LogLevel::FAILURE, "Received message:", configSet);
    Fatal::exit();
  }

  stateMachine.transition(&MasterStates::computeNextConfig);
}

void fallbackToPrevious(const State &current) {
  if (!loraHandler.canTransmit())
    return;

  serial.log(LogLevel::INFO,
             "Downgrading LoRa configuration to:", lastNodeConf);
  loraHandler.updateConfig(lastNodeConf);
  localNodeConf = lastNodeConf;

  stateMachine.transition(&MasterStates::computeNextConfig);
}

void waitFallbackConfigAck(const State &current) {
  static Timeout timeout;

  if (loraHandler.hasBeenRead()) {
    if (timeout.hasTimedOut()) {
      serial.log(LogLevel::ERROR, current, "Timed out");
      stateMachine.transition(&MasterStates::fallbackOnTimeout);
    }

    return;
  }

  timeout.reset();

  Message fallbackSet = loraHandler.getMessage();

  if (fallbackSet.type != MessageType::CONFIG_ACK) {
    serial.log(LogLevel::FAILURE, current, "Received wrong message type!");
    serial.log(LogLevel::FAILURE, "Received message:", fallbackSet);
    Fatal::exit();
  }

  stateMachine.transition(&MasterStates::initialState);
}

void fallbackOnTimeout(const State &current) {
  serial.log(LogLevel::WARNING, current, "Falling back to default config!");

  loraHandler.updateConfig(defaultConfig);
  localNodeConf = defaultConfig;

  stateMachine.transition(&MasterStates::initialState);
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

State MasterStates::computeNextConfig = {.name = "Compute next config",
                                         .action = Actions::computeNextConfig};

State MasterStates::sendConfigRequest = {.name = "Send Config Request",
                                         .action = Actions::sendConfigRequest};

State MasterStates::waitConfigAck = {.name = "Wait Config Ack",
                                     .action = Actions::waitConfigAck};

State MasterStates::fallbackToPrevious = {.name = "Fallback to previous config",
                                          .action =
                                              Actions::fallbackToPrevious};

State MasterStates::waitFallbackConfigAck = {
    .name = "Wait Fallback Config Set",
    .action = Actions::waitFallbackConfigAck};

State MasterStates::fallbackOnTimeout = {.name =
                                             "Fallback to default (Timeout)",
                                         .action = Actions::fallbackOnTimeout};

State MasterStates::sendConfigStart = {.name = "Send config start",
                                       .action = Actions::sendConfigStart};
State MasterStates::sendConfigEnd = {.name = "Send config end",
                                     .action = Actions::sendConfigEnd};

StateMachine stateMachine(&MasterStates::initialState);
