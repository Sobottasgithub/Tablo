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
    std::shared_ptr<arrow::ChunkedArray> getColumnByIndex(int index);
    std::shared_ptr<arrow::Table> getViewport(int xStart, int xEnd, int yStart, int yEnd);

  private:
    ttp2::ServerSessionController::File file;
};

#endif
