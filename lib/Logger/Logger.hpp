#include <initializer_list>
#include <types/LoraTypes.hpp>
#include <types/Message.hpp>

enum LogLevel { FAILURE, ERROR, WARNING, INFORMATION, STATISTICS, DEBUG };

class Logger {
public:
  Logger();
  ~Logger() = default;

  Logger(Logger &&) = delete;
  Logger(const Logger &) = delete;
  Logger &operator=(Logger &&) = delete;
  Logger &operator=(const Logger &) = delete;

  void log(LogLevel, const char *, const char *endStr = "\n");
  void log(LogLevel, const std::initializer_list<const char *> &,
           const char *separator = " ", const char *endStr = "\n");

  void log(LogLevel, const char *, Message);
  void log(LogLevel, const char *, LoRaConfig);
  void printLegend();
};

extern Logger serial;
