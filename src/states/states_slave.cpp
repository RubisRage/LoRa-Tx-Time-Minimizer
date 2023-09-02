#include "states/states_slave.hpp"
#include "LoRa.h"
#include "types/LoraTypes.hpp"
#include <LoraHandler/LoraHandler.hpp>

void testStateFunc() {

  Message message;

  if (!loraHandler.get(message)) {
    return;
  }

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

  serial.log(LogLevel::INFORMATION, "Remote node config:", remoteNodeConf);
  serial.log(LogLevel::INFORMATION, "Remote RSSI:", remoteRSSI,
             "dBm, Remote SNR:", remoteSNR, "dB");
}

constexpr State states[] = {
    State{.id = 0, .name = "Test state", .execute = testStateFunc}};

StateMachine<1, 0> slaveStateMachine(&states[0]);

StateMachine<1, 0> &initializeSlaveStateMachine() { return slaveStateMachine; }
