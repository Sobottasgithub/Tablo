#include "csv_manager.h"

#include <server_session_controller.h>

#include <iostream>
#include <algorithm>
#include <functional>
#include <string>

void CsvManager::setFile(ttp2::ServerSessionController::File newFile) {
  this->file = newFile;
  std::wcout << this->file.payload.c_str() << std::endl;
  std::wcout << "C: " << getColumnCount() << std::endl;
  std::wcout << "R: " << getRowCount() << std::endl;
  std::wcout << "SecondRow: " << getRowByIndex(2).c_str() << std::endl;
  std::wcout << "SecondColumn: " << getColumnByIndex(2).c_str() << std::endl;
}

std::string CsvManager::getFilePath() {
  return this->file.filePath;
}

int CsvManager::getRowCount() {
  int count = 0;
  for (int index = 0; index < this->file.payload.length(); index++)
      if (this->file.payload[index] == '\n')
           count++;
  return count;
}

int CsvManager::getColumnCount() {
  // get position of first linebreak
  std::string delimiter = "\n";
  const std::boyer_moore_searcher searcher(delimiter.begin(), delimiter.end());
  const auto distanceToDelimiter = std::search(this->file.payload.begin(), this->file.payload.end(), searcher);
  if (distanceToDelimiter != this->file.payload.end()) {
    // substring first row
    std::string firstRow = this->file.payload.substr(0, std::distance(this->file.payload.begin(), distanceToDelimiter));

    int count = 1;
    for (int index = 0; index < firstRow.length(); index++)
        if (firstRow[index] == ',')
            count++;
    return count;
  } else
    return 0;
}

std::string CsvManager::getRowByIndex(int index) {
  std::string resultRow = "";

  if (index < 1) {
    return resultRow;
  } else if (index <= getRowCount()) {
    int count = 0;
    if (index == 1) {
      for (int countIndex = 0; countIndex < this->file.payload.length(); countIndex++) {
          if (this->file.payload[countIndex] == '\n')
               return resultRow;
          resultRow = resultRow + this->file.payload[countIndex];
      } 
    }
    
    for (int countIndex = 0; countIndex < this->file.payload.length(); countIndex++) {
        if (this->file.payload[countIndex] == '\n') {
             if (count < index)
               count++;
             if (count == index)
               break;
        } else if (count == index-1) {
          resultRow = resultRow + this->file.payload[countIndex];
        }
    }
    return resultRow;
  } else {
    return resultRow;
  }
}

std::string CsvManager::getColumnByIndex(int index) {
  std::string resultRow;

  int column = 0;
  for (int countIndex = 0; countIndex < this->file.payload.length(); countIndex++) {
      if (this->file.payload[countIndex] == '\n') {
           resultRow = resultRow + '\n';
           column = 0;
      } else if (this->file.payload[countIndex] == ',') {
        column++;
        continue;
      }
      if (column == index-1) {
        resultRow = resultRow + this->file.payload[countIndex];
      }
  }

  return resultRow;
}
