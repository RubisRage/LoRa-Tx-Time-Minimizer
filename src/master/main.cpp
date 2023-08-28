#include "Arduino.h"
#include <cstdint>
#include <stdlib.h>

int status = 1;

void setup() {
    SerialUSB.begin(9600);
}

    
void loop() {
    while (true) {
        SerialUSB.println("string");
    }
}
