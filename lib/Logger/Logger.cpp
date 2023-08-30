#include "Logger.hpp"
#include <Arduino.h>
#include <algorithm>
#include <string>

static const char *levelIndicators[] = {
    "(FF)", /* LogLevel::FAILURE */
    "(EE)", /* LogLevel::ERROR */
    "(WW)", /* LogLevel::WARNING */
    "(II)", /* LogLevel::INFORMATION */
    "(DD)", /* LogLevel::DEBUG */
};

static const char *levelLegends[] = {
    "Complete failure", /* LogLevel::FAILURE */
    "Error",            /* LogLevel::ERROR */
    "Warning",          /* LogLevel::WARNING */
    "Information",      /* LogLevel::INFORMATION */
    "Debug",            /* LogLevel::INFORMATION */
};

Logger::Logger() {}

void Logger::log(LogLevel level, const char *logMessage, const char *end) {
  Serial.print(levelIndicators[level]);
  Serial.print(" -> ");
  Serial.print(logMessage);
  Serial.print(end);
}

void Logger::log(LogLevel level,
                 std::initializer_list<const char *> logMessages,
                 const char *separator, const char *end) {
  Serial.print(levelIndicators[level]);
  Serial.print(" -> ");

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

  Serial.println("\tSender address: 0x" + String(message.sourceAddress, HEX));
  Serial.println("\tRecipient address: 0x" +
                 String(message.destinationAddress, HEX));
  Serial.println("\tMessage ID:" + String(message.id));
  Serial.println("\tPayload length:" + String(message.payloadLength));
  Serial.print("\tPayload:");

  for (int i = 0; i < message.payloadLength; i++) {
    Serial.print(message.payload[i], HEX);
    Serial.print(" ");
  }

  Serial.println();
}

void Logger::log(LogLevel level, const char *logMessage, LoRaConfig config) {
  log(level, logMessage, " ");

  Serial.print("{ BW: ");
  Serial.print(bandwidth_kHz[config.bandwidthIndex]);
  Serial.print(" kHz, SPF: ");
  Serial.print(config.spreadingFactor);
  Serial.print(", CR: ");
  Serial.print(config.codingRate);
  Serial.print(", TxPwr: ");
  Serial.print(config.txPower);
  Serial.println("}");
}

void Logger::printLegend() {
  for (size_t i = 0; i < sizeof(levelIndicators) / sizeof(const char *); i++) {
    Serial.print(levelIndicators[i]);
    Serial.print(": ");
    Serial.println(levelLegends[i]);
  }
}

/* Global declarations (extern) */
Logger serial;
