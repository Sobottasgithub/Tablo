#include "../include/tablog.h"

#include <iostream>

namespace tablog {
  Tablog::Tablog() {}
  Tablog::Tablog(std::string name, bool displayTimestamp) {
    configure(name, displayTimestamp);
  }
   
  void Tablog::configure(std::string name, bool displayTimestamp) {
     this->name = name;
     this->displayTimestamp = displayTimestamp;
  }
   
  void Tablog::log(LogLevel loglevel, const std::string& message) {
     std::cout << "TABLOG::: " << message.c_str() << std::endl;
  }

  std::string Tablog::logLevelToString(LogLevel level) {
    switch (level) {
      case DEBUG:
        return "DEBUG";
      case INFO:
          return "INFO";
      case WARNING:
          return "WARNING";
      case ERROR:
          return "ERROR";
      case CRITICAL:
          return "CRITICAL";
      default:
          return "UNKNOWN";
      }
  }
}
