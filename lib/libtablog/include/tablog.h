#ifndef libtablog
#define libtablog

#include <string>
#include <mutex>

namespace tablog {
 enum LogLevel { DEBUG, INFO, WARNING, ERROR, CRITICAL };
 
 class Tablog {
  public:
   Tablog(const Tablog& object) = delete;
   static Tablog* getInstance();
   
   void configure(std::string name, bool displayTimestamp);
   void log(LogLevel loglevel, const std::string& message);
   
  private:
   static Tablog* instance;
   static std::mutex mtx;
   Tablog();

   std::string name = "";
   bool displayTimestamp = true;

   std::string logLevelToString(LogLevel level);
 };
}

#endif
