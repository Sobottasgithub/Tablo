#ifndef NODE_EXECUTION_ENDPOINT_H
#define NODE_EXECUTION_ENDPOINT_H

#include <execution_endpoint.h>
#include <memory>
#include <server_session_controller.h>

#include <memory.h>
#include <arrow/table.h>

class NodeExecutionEndpoint: public tql::ExecutionEndpoint {
  public:
    std::shared_ptr<arrow::Table> openFile(std::string filePath) = delete;
    
    void registerFile(ttp2::ServerSessionController::File file);
    void setQueryResult(ttp2::ServerSessionController::File queryResultTable);
    std::shared_ptr<arrow::Table> setSelectedFile(std::string filePath);
    std::shared_ptr<arrow::Table> getFile(std::string filePath);
    ttp2::ServerSessionController::File* getSelectedFile();

  private:
    ttp2::ServerSessionController::File* selectedFile = nullptr;

    ttp2::ServerSessionController::File queryResultTable;
    std::vector<ttp2::ServerSessionController::File> currentFiles = {};
};

#endif
