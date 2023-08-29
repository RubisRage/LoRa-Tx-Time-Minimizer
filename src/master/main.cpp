#include "lora.hpp"
#include <Arduino.h>
#include <ArduinoClock.hpp>
#include <DutyCycleManager.hpp>
#include <LoRa.h>
#include <SPI.h>
#include <algorithm>
#include <array>
#include <cstdint>
#include <string>

using namespace std::chrono_literals;

#define INITIAL_INTERVAL_BETWEEN_TX 10000ms

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

  if (packetSize == 0)
    return;

  Message message;

  message.destinationAddress = LoRa.read();
  message.sourceAddress = LoRa.read();
  message.id = ((uint16_t)LoRa.read() << 7) | (uint16_t)LoRa.read();
  message.type = MessageType(LoRa.read());
  message.payloadLength = LoRa.read();

  uint8_t payload[10];
  message.payload = payload;

  uint8_t receivedBytes = 0;
  while (LoRa.available() && (receivedBytes < uint8_t(sizeof(payload) - 1))) {
    payload[receivedBytes++] = (char)LoRa.read();
  }

  if (message.payloadLength != receivedBytes) {
    Serial.print("Receiving error: declared message length " +
                 String(message.payloadLength));
    Serial.println(" does not match length " + String(receivedBytes));
    return;
  }

  if ((message.destinationAddress & localAddress) != localAddress) {
    Serial.println("Receiving error: This message is not for me.");
    return;
  }

  Serial.println("Received from: 0x" + String(message.sourceAddress, HEX));
  Serial.println("Sent to: 0x" + String(message.destinationAddress, HEX));
  Serial.println("Message ID: " + String(message.id));
  Serial.println("Payload length: " + String(message.payloadLength));
  Serial.print("Payload: ");
  printBinaryPayload(message.payload, receivedBytes);
  Serial.print("\nRSSI: " + String(LoRa.packetRssi()));
  Serial.print(" dBm\nSNR: " + String(LoRa.packetSnr()));
  Serial.println(" dB");

  if (receivedBytes == 4) {
    remoteNodeConf.bandwidthIndex = payload[0] >> 4;
    remoteNodeConf.spreadingFactor = 6 + ((payload[0] & 0x0F) >> 1);
    remoteNodeConf.codingRate = 5 + (payload[1] >> 6);
    remoteNodeConf.txPower = 2 + ((payload[1] & 0x3F) >> 1);
    remoteRSSI = -int(payload[2]) / 2.0f;
    remoteSNR = int(payload[3]) - 148;

    Serial.print("Remote config: BW: ");
    Serial.print(bandwidth_kHz[remoteNodeConf.bandwidthIndex]);
    Serial.print(" kHz, SPF: ");
    Serial.print(remoteNodeConf.spreadingFactor);
    Serial.print(", CR: ");
    Serial.print(remoteNodeConf.codingRate);
    Serial.print(", TxPwr: ");
    Serial.print(remoteNodeConf.txPower);
    Serial.print(" dBm, RSSI: ");
    Serial.print(remoteRSSI);
    Serial.print(" dBm, SNR: ");
    Serial.print(remoteSNR, 1);
    Serial.println(" dB\n");
  } else {
    Serial.print("Unexpected payload size: ");
    Serial.print(receivedBytes);
    Serial.println(" bytes\n");
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;

  setupLora(localNodeConf, onReceive, onTxDone);
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
  static DutyCycleManager dutyCycleManager(INITIAL_INTERVAL_BETWEEN_TX);
  static uint16_t msgCount = 0;

  if (!transmitting && dutyCycleManager.canTransmit()) {

    // uint8_t payload[50];
    size_t payloadLength = 0;

    std::array<uint8_t, 50> payload;

    buildPayload(payload, payloadLength);

    transmitting = true;
    txDoneFlag = false;
    dutyCycleManager.beginTx();

    Message message;
    message.id = msgCount;
    message.type = MessageType::STATUS;
    message.sourceAddress = localAddress;
    message.destinationAddress = remoteAddress;
    message.payload = payload.data();
    message.payloadLength = payloadLength;

    sendMessage(message);
    Serial.print("Sending packet ");
    Serial.print(msgCount++);
    Serial.print(": ");

    std::for_each(payload.begin(), payload.begin() + payloadLength,
                  [](uint8_t e) {
                    Serial.print(e, HEX);
                    Serial.print(" ");
                  });
  }

  if (doneTransmitting(transmitting, txDoneFlag)) {

    dutyCycleManager.updateIntervalBetweenTx();

    Serial.print("Duty cycle: ");
    Serial.println(dutyCycleManager.dutyCycle);

    transmitting = false;

    /* Enable receiving, which is disabled when transmitting */
    LoRa.receive();
  }
}
