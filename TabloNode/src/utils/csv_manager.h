#ifndef CSV_MANAGER_H
#define CSV_MANAGER_H

#include <server_session_controller.h>

#include <tablog_registry.h>
#include <tablog.h>

#include <string>

class CsvManager {
  public:
    void setFile(ttp2::ServerSessionController::File newFile);
    std::string getFilePath();
    int getRowCount();
    int getColumnCount();
    std::shared_ptr<arrow::ChunkedArray> getColumnByIndex(int index);
    std::shared_ptr<arrow::Table> getViewport(int xStart, int xEnd, int yStart, int yEnd);
    std::shared_ptr<arrow::Table> filter(std::string columnName, std::string regex);

  private:
    std::shared_ptr<tablog::Tablog> logger = tablog::TablogRegistry::getInstance().get("Tablo-Node");
        
    ttp2::ServerSessionController::File file;

    bool applyRegexOnScalar(const std::shared_ptr<arrow::Scalar>& scalar, const std::string regex);
};

#endif
