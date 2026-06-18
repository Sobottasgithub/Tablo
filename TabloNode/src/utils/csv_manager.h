#ifndef CSV_MANAGER_H
#define CSV_MANAGER_H

#include <server_session_controller.h>

#include <string>

class CsvManager {
  public:
    void setFile(ttp2::ServerSessionController::File newFile);
    std::string getFilePath();
    int getRowCount();
    int getColumnCount();
    std::string getRowByIndex(int index);
    std::string getColumnByIndex(int index);
    std::string getColumnByIndex(int index, std::string rows);
    std::string getViewport(int xStart, int xEnd, int yStart, int yEnd);

  private:
    ttp2::ServerSessionController::File file;
};

#endif
