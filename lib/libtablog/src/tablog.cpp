#include "../include/tablog.h"

#include <iostream>
#include <ctime>

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
     std::cout << "<" << name << "> " << "[" << logLevelToString(loglevel) << "] ";
     if (displayTimestamp) {
       time_t now = time(0);
        tm* timeinfo = localtime(&now);
        char timestamp[20];
        strftime(timestamp, sizeof(timestamp),
                 "%H:%M:%S", timeinfo);
        std::cout << timestamp << " ";
     }
     std::cout << message.c_str() << std::endl;
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
