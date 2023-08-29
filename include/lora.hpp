#include <LoRa.h>

double bandwidth_kHz[10] = {7.8E3,  10.4E3, 15.6E3, 20.8E3, 31.25E3,
                            41.7E3, 62.5E3, 125E3,  250E3,  500E3};
struct LoRaConfig {
  uint8_t bandwidthIndex;
  uint8_t spreadingFactor;
  uint8_t codingRate;
  uint8_t txPower;
};

LoRaConfig defaultConfig = {
    .bandwidthIndex = 6, .spreadingFactor = 10, .codingRate = 5, .txPower = 2};

enum MessageType : uint8_t {
  STATUS,
  CONFIG_REQ,
  ACK,
};

struct Message {
  uint16_t id;
  MessageType type;
  uint8_t sourceAddress;
  uint8_t destinationAddress;
  uint8_t *payload;
  uint8_t payloadLength;
};

void printBinaryPayload(uint8_t *payload, uint8_t payloadLength) {
  for (int i = 0; i < payloadLength; i++) {
    Serial.print((payload[i] & 0xF0) >> 4, HEX);
    Serial.print(payload[i] & 0x0F, HEX);
    Serial.print(" ");
  }
}

void sendMessage(Message &message) {
  while (!LoRa.beginPacket()) {
    delay(10);
  }
  LoRa.write(message.destinationAddress);
  LoRa.write(message.sourceAddress);
  LoRa.write((uint8_t)(message.messageId >> 7));
  LoRa.write((uint8_t)(message.messageId & 0xFF));
  LoRa.write(message.type);
  LoRa.write(message.payloadLength);
  LoRa.write(message.payload, (size_t)message.payloadLength);
  LoRa.endPacket(true);
}

void setupLora(LoRaConfig &config, void (*onReceive)(int), void (*onTxDone)()) {
  if (!LoRa.begin(868E6)) {
    Serial.println("LoRa init failed. Check your connections.");
    while (true)
      ;
  }

  LoRa.setSignalBandwidth(long(bandwidth_kHz[config.bandwidthIndex]));
  LoRa.setSpreadingFactor(config.spreadingFactor);
  LoRa.setCodingRate4(config.codingRate);
  LoRa.setTxPower(config.txPower, PA_OUTPUT_PA_BOOST_PIN);
  LoRa.setSyncWord(0x12);
  LoRa.setPreambleLength(8);
  LoRa.onReceive(onReceive);
  LoRa.onTxDone(onTxDone);
  LoRa.receive();

  Serial.println("LoRa init succeeded.\n");
}
