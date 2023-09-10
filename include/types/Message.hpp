#pragma once

#include <cstdint>
#include <globals/globals.hpp>

enum MessageType : uint8_t {
  UNINITIALIZED,
  CONFIG_START,
  CONFIG_END,
  CONFIG_SET,
  CONFIG_ACK,
  ECHO_REQ,
  ECHO_REPLY
};

constexpr std::array<const char *, 7> messageTypesNames = {
    "UNINITIALIZED", "CONFIG_START", "CONFIG_END", "CONFIG_SET",
    "CONFIG_ACK",    "ECHO_REQ",     "ECHO_REPLY",
};

struct Message {
  uint16_t id = 0;
  MessageType type = MessageType::UNINITIALIZED;
  uint8_t sourceAddress = 0;
  uint8_t destinationAddress = 0;
  uint8_t *payload = nullptr;
  uint8_t payloadLength = 0;

  Message() = default;
  Message(const Message &) = default;
  Message(Message &) = default;
  Message(volatile Message &other) {
    this->destinationAddress = other.destinationAddress;
    this->sourceAddress = other.sourceAddress;
    this->id = other.id;
    this->type = other.type;
    this->payload = other.payload;
    this->payloadLength = other.payloadLength;
  };

  volatile Message &operator=(Message &other) volatile {
    this->destinationAddress = other.destinationAddress;
    this->sourceAddress = other.sourceAddress;
    this->id = other.id;
    this->type = other.type;
    this->payload = other.payload;
    this->payloadLength = other.payloadLength;

    return *this;
  };

  Message(uint16_t count, MessageType type)
      : id(idPrefix | count), type(type), sourceAddress(localAddress),
        destinationAddress(remoteAddress), payload(nullptr), payloadLength(0) {}

  static constexpr ssize_t headerSize() {
    return sizeof(uint8_t) * 3 + sizeof(MessageType) + sizeof(uint16_t);
  }
};
