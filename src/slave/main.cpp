#include "Arduino.h"
#include "lora.hpp"
#include <LoRa.h>
#include <SPI.h>
#include <ArduinoClock.hpp>

void setup() {
  Serial.begin(9600);
  while (!Serial) ;

}

void loop() {
  Serial.println(ArduinoClock::now().time_since_epoch().count());

  delay(1000);
}
