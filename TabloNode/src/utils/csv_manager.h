#ifndef CSV_MANAGER_H
#define CSV_MANAGER_H

#include <server_session_controller.h>

#include <string>

class CsvManager {
  public:
    void setFile(ttp2::ServerSessionController::File newFile);
    std::string getFilePath();

  private:
    ttp2::ServerSessionController::File file;

};

#endif
