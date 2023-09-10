#include <Arduino.h>
#include <LoRa.h>
#include <LoraHandler/LoraHandler.hpp>

#include "states.hpp"
#include "variant.h"

void setup() {
  Serial.begin(115200);
  while (!Serial) {
  }

  serial.printLegend();
  serial.log(LogLevel::INFO, "Press s to start execution.");

  while (Serial.read() != 's') {
  }

  if (!LoRa.begin(868E6)) {
    serial.log(LogLevel::FAILURE, "LoRa init failed. Check your connections.");
    Fatal::exit();
  }

  loraHandler.setup(defaultConfig, nullptr);

#ifdef MASTER_BOARD
  serial.log(LogLevel::INFO, "MASTER SETUP CORRECTLY");
#endif

#ifdef SLAVE_BOARD
  serial.log(LogLevel::INFO, "SLAVE SETUP CORRECTLY");
#endif
}

void loop() { stateMachine.getState().execute(); }
