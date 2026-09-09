#include "node_execution_endpoint.h"
#include <stdexcept>
#include <iostream>

void NodeExecutionEndpoint::registerFile(ttp2::ServerSessionController::File file) {
  for (int index = 0; index < currentFiles.size(); index++) {
    // Update file if it already exists
    if (currentFiles.at(index).filePath == file.filePath) {
      currentFiles[index] = file;
      selectedFile = &currentFiles[index];
      return;
    }
  }
  currentFiles.push_back(file);
  selectedFile = &currentFiles.back();
}

std::shared_ptr<arrow::Table> NodeExecutionEndpoint::setSelectedFile(std::string filePath) {
  for (int index = 0; index < currentFiles.size(); index++) {
    std::cout << "-------------------FILEPATH" << currentFiles.at(index).filePath << std::endl;
    if (currentFiles.at(index).filePath.compare(filePath) == 0) {
      selectedFile = &currentFiles.at(index);

      std::cout << "-------------------FILE" << selectedFile->payload << std::endl; 
      
      return selectedFile->payload;
    }
  }
  throw std::invalid_argument("Unknown File path!");
}

ttp2::ServerSessionController::File* NodeExecutionEndpoint::getSelectedFile() {
  return selectedFile;  
}
