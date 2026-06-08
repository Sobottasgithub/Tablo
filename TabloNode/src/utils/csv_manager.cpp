#include "csv_manager.h"

#include <server_session_controller.h>

void CsvManager::setFile(ttp2::ServerSessionController::File newFile) {
  this->file = newFile;
}

std::string CsvManager::getFilePath() {
  return this->file.filePath;
}
