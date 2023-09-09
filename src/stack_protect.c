#include <Arduino.h>

void *__stack_chk_guard = (void *)0xdeadbeef;

void __stack_chk_fail(void) {
  digitalWrite(LED_BUILTIN, HIGH);
  while (true)
    ;
}
