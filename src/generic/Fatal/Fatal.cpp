#include <Fatal/Fatal.hpp>
#include <Logger/Logger.hpp>

void Fatal::exit() {
  serial.log(LogLevel::FAILURE, "Now exiting.");

  while (true) {
  }
}
