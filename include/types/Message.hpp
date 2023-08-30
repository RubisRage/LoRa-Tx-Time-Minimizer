#pragma once

#include <cstdint>

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
