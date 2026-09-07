#include "node_execution_endpoint.h"
#include <stdexcept>

void NodeExecutionEndpoint::registerFile(ttp2::ServerSessionController::File file) {
  
}

std::shared_ptr<arrow::Table> NodeExecutionEndpoint::setSelectedFile(std::string filePath) {
  for (int index = 0; index < currentFiles.size(); index++) {
    if (currentFiles.at(index).filePath == filePath) {
      selectedFile = &currentFiles.at(index);
      return selectedFile->payload;
    }
  }
  throw std::invalid_argument("Unknown File path!");
}

ttp2::ServerSessionController::File* NodeExecutionEndpoint::getSelectedFile() {
  return selectedFile;  
}
