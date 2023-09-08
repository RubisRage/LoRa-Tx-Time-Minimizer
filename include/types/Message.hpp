#pragma once

#include <cstdint>
#include <globals/globals.hpp>

enum MessageType : uint8_t {
  STATUS,
  CONFIG_REQ,
  ECHO_REQ,
  ECHO_REPLY,
  FALLBACK_REQ,
  ACK,
};

struct Message {
  uint16_t id;
  MessageType type;
  uint8_t sourceAddress;
  uint8_t destinationAddress;
  uint8_t *payload;
  uint8_t payloadLength;

  Message() = default;
  Message(uint16_t count, MessageType type)
      :
#ifdef MASTER_BOARD
        id(0x8 & count),
#endif
#ifdef SLAVE_BOARD
        id(count),
#endif
        type(type), sourceAddress(localAddress),
        destinationAddress(remoteAddress), payload(nullptr), payloadLength(0) {
  }
};
