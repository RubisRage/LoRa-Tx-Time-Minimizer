#include <Arduino.h>
#include <ArduinoClock.hpp>
#include <LoRa.h>
#include <Logger.hpp>
#include <LoraHandler.hpp>
#include <SPI.h>
#include <algorithm>
#include <array>
#include <cstdint>
#include <string>

const uint8_t localAddress = 0xB0;
uint8_t remoteAddress = 0xFF;

volatile bool txDoneFlag = true;
volatile bool transmitting = false;

LoRaConfig localNodeConf = defaultConfig;
LoRaConfig remoteNodeConf;

int remoteRSSI = 0;
float remoteSNR = 0;

void onTxDone() { txDoneFlag = true; }

void onReceive(int packetSize) {
  if (transmitting && !txDoneFlag)
    txDoneFlag = true;

  if (packetSize == 0) {
    serial.log(LogLevel::WARNING, "\"Received\" empty packet.");
    return;
  }

  Message message;

  message.destinationAddress = LoRa.read();
  message.sourceAddress = LoRa.read();
  message.id = ((uint16_t)LoRa.read() << 7) | (uint16_t)LoRa.read();
  message.type = MessageType(LoRa.read());
  message.payloadLength = LoRa.read();

  std::array<uint8_t, 10> payload;
  message.payload = payload.data();

  uint8_t receivedBytes = 0;
  while (LoRa.available() && (receivedBytes < uint8_t(payload.size() - 1)) &&
         receivedBytes < message.payloadLength) {
    payload[receivedBytes++] = (char)LoRa.read();
  }

  while (LoRa.available())
    LoRa.read();

  if (message.payloadLength != receivedBytes) {
    serial.log(LogLevel::ERROR,
               {"Receiving error: declared message length ",
                String(message.payloadLength).c_str(),
                " does not match length ", String(receivedBytes).c_str()});
    return;
  }

  if ((message.destinationAddress & localAddress) != localAddress) {
    serial.log(LogLevel::WARNING,
               "Received message was not meant for localhost, dropping.");
    return;
  }

  serial.log(LogLevel::INFORMATION, "Received message", message);

  serial.log(LogLevel::INFORMATION,
             {"Local RSSI:", String(LoRa.packetRssi()).c_str(),
              "dBm, Local SNR:", String(LoRa.packetSnr()).c_str(), "dB"});

  if (receivedBytes == 4) {
    remoteNodeConf.bandwidthIndex = payload[0] >> 4;
    remoteNodeConf.spreadingFactor = 6 + ((payload[0] & 0x0F) >> 1);
    remoteNodeConf.codingRate = 5 + (payload[1] >> 6);
    remoteNodeConf.txPower = 2 + ((payload[1] & 0x3F) >> 1);
    remoteRSSI = -int(payload[2]) / 2.0f;
    remoteSNR = int(payload[3]) - 148;

    serial.log(LogLevel::INFORMATION, "Remote node config:", remoteNodeConf);
    serial.log(LogLevel::INFORMATION,
               {"Remote RSSI:", String(remoteRSSI).c_str(),
                "dBm, Remote SNR:", String(remoteRSSI).c_str(), "dB"});
  } else {
    serial.log(LogLevel::ERROR, {"Unexpected payload size: ",
                                 String(receivedBytes).c_str(), "bytes"});
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;

  while (Serial.read() != 's')
    ;

  if (!LoRa.begin(868E6)) {
    serial.log(LogLevel::FAILURE, "LoRa init failed. Check your connections.");
    while (true)
      ;
  }

  loraHandler.setup(localNodeConf, onReceive);

  serial.log(LogLevel::INFORMATION, "MASTER SETUP CORRECTLY");
}

template <size_t size>
void buildPayload(std::array<uint8_t, size> payload, size_t &payloadLength) {
  payload[payloadLength] = (localNodeConf.bandwidthIndex << 4);
  payload[payloadLength++] |= ((localNodeConf.spreadingFactor - 6) << 1);
  payload[payloadLength] = ((localNodeConf.codingRate - 5) << 6);
  payload[payloadLength++] |= ((localNodeConf.txPower - 2) << 1);
  payload[payloadLength++] = uint8_t(-LoRa.packetRssi() * 2);
  payload[payloadLength++] = uint8_t(148 + LoRa.packetSnr());
}

bool doneTransmitting(bool transmitting, bool txDoneFlag) {
  return transmitting && txDoneFlag;
}

void loop() {
  static uint16_t msgCount = 0;

  if (loraHandler.canTransmit()) {

    size_t payloadLength = 0;

    std::array<uint8_t, 50> payload;

    buildPayload(payload, payloadLength);

    transmitting = true;
    txDoneFlag = false;

    Message message;
    message.id = msgCount;
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
