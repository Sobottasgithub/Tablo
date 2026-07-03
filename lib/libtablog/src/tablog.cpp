#include "../include/tablog.h"

#include <iostream>
#include <ctime>
#include <mutex>

namespace tablog {  
  Tablog& Tablog::getInstance() {
      static Tablog instance; 
      return instance;
  }

  void Tablog::configure(std::string name, bool displayTimestamp) {
     this->name = name;
     this->displayTimestamp = displayTimestamp;
  }
   
  void Tablog::log(LogLevel loglevel, const std::string& message) {
     std::cout << "<\033[1;34m" << name << "\033[0m> " << "[" << logLevelToString(loglevel) << "] ";
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
      // https://en.wikipedia.org/wiki/ANSI_escape_code
      case DEBUG:
        return "\033[32mDEBUG\033[0m";
      case INFO:
          return "\033[36mINFO\033[0m";
      case WARNING:
          return "\033[93mWARNING\033[0m";
      case ERROR:
          return "\033[41;30mERROR\033[0m";
      case CRITICAL:
          return "\033[95mCRITICAL\033[0m";
      default:
          return "UNKNOWN";
      }
  }
}
