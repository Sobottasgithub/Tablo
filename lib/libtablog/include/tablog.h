#ifndef libtablog
#define libtablog

#include <string>

namespace tablog {
 enum LogLevel { DEBUG, INFO, WARNING, ERROR, CRITICAL };
 
 class Tablog {
  public:
   Tablog();
   Tablog(std::string name, bool displayTimestamp);
   void configure(std::string name, bool displayTimestamp);
   void log(LogLevel loglevel, const std::string& message);
   
  private:
   std::string name = "";
   bool displayTimestamp = true;

   std::string logLevelToString(LogLevel level);
 };
}

#endif
