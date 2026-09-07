#ifndef CSV_MANAGER_H
#define CSV_MANAGER_H

#include "node_execution_endpoint.h"

#include <server_session_controller.h>
#include <interpreter.h>
#include <parser.h>

#include <tablog_registry.h>
#include <tablog.h>

#include <string>

class CsvManager {
  public:
    CsvManager();
    
    void setFile(ttp2::ServerSessionController::File newFile);
    std::string getFilePath();
    int getRowCount();
    int getColumnCount();
    std::shared_ptr<arrow::ChunkedArray> getColumnByIndex(int index);
    std::shared_ptr<arrow::Table> getViewport(int xStart, int xEnd, int yStart, int yEnd);
    std::shared_ptr<arrow::Table> executeQuery(const std::string& query);

  private:
    std::shared_ptr<tablog::Tablog> logger = tablog::TablogRegistry::getInstance().get("Tablo-Node");

    NodeExecutionEndpoint executionEndpoint;
    tql::Parser parser;
    tql::Interpreter interpreter;
};

#endif
