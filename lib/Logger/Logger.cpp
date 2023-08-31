#include "Logger.hpp"
#include "types/LoraTypes.hpp"
#include <Arduino.h>
#include <algorithm>
#include <string>

struct Level {
  const char *label;
  const char *legend;
};

static std::array<Level, 6> levels = {
    Level{"(FF)", "Complete failure"}, /* LogLevel::FAILURE */
    Level{"(EE)", "Error"},            /* LogLevel::ERROR */
    Level{"(WW)", "Warning"},          /* LogLevel::WARNING */
    Level{"(II)", "Information"},      /* LogLevel::INFORMATION */
    Level{"(SS)", "Statistics"},       /* LogLevel::STATISTICS */
    Level{"(DD)", "Debug"}             /* LogLevel::DEBUG */
};

Logger::Logger() {}

void printLabel(LogLevel level) {

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

void Logger::log(LogLevel level,
                 const std::initializer_list<const char *> &logMessages,
                 const char *separator, const char *end) {
  printLabel(level);

  auto current = logMessages.begin();

  while (current != logMessages.end()) {
    Serial.print(*current);

    if (current != logMessages.end() - 1)
      Serial.print(separator);

    current++;
  }

  Serial.print(end);
}

void Logger::log(LogLevel level, const char *logMessage, Message message) {

  log(level, logMessage);

  Serial.println("{");
  Serial.println("\t\tSender address: 0x" + String(message.sourceAddress, HEX));
  Serial.println("\t\tRecipient address: 0x" +
                 String(message.destinationAddress, HEX));
  Serial.println("\t\tMessage ID: " + String(message.id));
  Serial.println("\t\tPayload length: " + String(message.payloadLength));
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
