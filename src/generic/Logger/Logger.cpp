#include "types/LoraTypes.hpp"
#include <Arduino.h>
#include <Logger/Logger.hpp>
#include <algorithm>
#include <string>

struct Level {
  const char *label;
  const char *legend;
};

static std::array<Level, 7> levels = {
    Level{"(FF)", "Complete failure"}, /* LogLevel::FAILURE */
    Level{"(EE)", "Error"},            /* LogLevel::ERROR */
    Level{"(WW)", "Warning"},          /* LogLevel::WARNING */
    Level{"(II)", "Information"},      /* LogLevel::INFORMATION */
    Level{"(SS)", "Statistics"},       /* LogLevel::STATISTICS */
    Level{"(TT)", "State transition"}, /* LogLevel::DEBUG */
    Level{"(DD)", "Debug"}             /* LogLevel::DEBUG */
};

Logger::Logger() {}

void Logger::printLabel(LogLevel level) {

  if (level < levels.size()) {
    Serial.print(levels[level].label);
  } else {
    Serial.print("[Unexpected LogLevel]");
  }

  Serial.print(" -> ");
}

void Logger::log(LogLevel level, const char *logMessage, const char *end) {
  printLabel(level);
  Serial.print(logMessage);
  Serial.print(end);
}

void Logger::log(LogLevel level, const char *logMessage, Message message) {
  log(level, logMessage);

  const char *types[] = {
      "STATUS", "CONFIG_REQ", "ECHO_REQ", "ECHO_REPLY", "FALLBACK_REQ", "ACK",
  };

  Serial.println("{");
  Serial.print("\t\tSender address: 0x");
  Serial.println(message.sourceAddress, HEX);
  Serial.print("\t\tRecipient address: 0x");
  Serial.println(message.destinationAddress, HEX);
  Serial.print("\t\tMessage ID: 0x");
  Serial.println(message.id, HEX);
  Serial.print("\t\tMessage type: ");
  Serial.println(types[message.type]);
  Serial.print("\t\tPayload length: ");
  Serial.println(message.payloadLength);
  Serial.print("\t\tPayload: ");

  for (int i = 0; i < message.payloadLength; i++) {
    Serial.print(message.payload[i], HEX);
    Serial.print(" ");
  }

  Serial.println("\n}");
}

void Logger::log(LogLevel level, const char *logMessage, LoRaConfig config) {
  log(level, logMessage, " ");

  Serial.print("{ BW: ");

  if (config.bandwidthIndex < bandwidth_kHz.size()) {
    Serial.print(bandwidth_kHz[config.bandwidthIndex]);
  } else {
    Serial.print("[Unexpected bandwidth index]");
  }

  Serial.print(" kHz, SPF: ");
  Serial.print(config.spreadingFactor);
  Serial.print(", CR: ");
  Serial.print(config.codingRate);
  Serial.print(", TxPwr: ");
  Serial.print(config.txPower);
  Serial.println("}");
}

void Logger::printLegend() {
  Serial.println("Log legend\n");

  for (size_t i = 0; i < sizeof(levels) / sizeof(Level); i++) {
    Serial.print("\t\t");
    Serial.print(levels[i].label);
    Serial.print(": ");
    Serial.println(levels[i].legend);
  }
}

/* Global declarations (extern) */
Logger serial;
