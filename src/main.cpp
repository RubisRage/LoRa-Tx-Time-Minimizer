#include <Arduino.h>
#include <LoRa.h>
#include <LoraHandler/LoraHandler.hpp>

#include "states.hpp"

void setup() {
  Serial.begin(115200);
  while (!Serial) {
  }

  serial.printLegend();
  serial.log(LogLevel::INFORMATION, "Press s to start execution.");

  while (Serial.read() != 's') {
  }

  if (!LoRa.begin(868E6)) {
    serial.log(LogLevel::FAILURE, "LoRa init failed. Check your connections.");
    Fatal::exit();
  }

  loraHandler.setup(defaultConfig, nullptr);

#ifdef MASTER_BOARD
  serial.log(LogLevel::INFORMATION, "MASTER SETUP CORRECTLY");
#endif

#ifdef SLAVE_BOARD
  serial.log(LogLevel::INFORMATION, "SLAVE SETUP CORRECTLY");
#endif
}

void loop() {
  stateMachine.getState().execute();
  loraHandler.updateTransmissionState();
}
