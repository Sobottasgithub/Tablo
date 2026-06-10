#include "csv_manager.h"

#include <server_session_controller.h>

#include <iostream>
#include <algorithm>
#include <functional>
#include <string>

void CsvManager::setFile(ttp2::ServerSessionController::File newFile) {
  this->file = newFile;
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
  return getColumnByIndex(index, this->file.payload);
}

std::string CsvManager::getColumnByIndex(int index, std::string rows) {
  std::string resultRow;

  int column = 0;
  for (int countIndex = 0; countIndex < rows.length(); countIndex++) {
      if (rows[countIndex] == '\n') {
           resultRow = resultRow + '\n';
           column = 0;
           continue;
      } else if (rows[countIndex] == ',') {
        column++;
        continue;
      }
      if (column == index-1) {
        resultRow = resultRow + rows[countIndex];
      }
  }

  return resultRow;
}

std::string CsvManager::getViewport(int xStart, int xEnd, int yStart, int yEnd) {
    std::string resultPayload = "";
    for (int xIndex = xStart; xIndex <= xEnd; xIndex++) {
        std::string currentRow = getRowByIndex(xIndex);

        std::string resultRow = "";
        for (int yIndex = yStart; yIndex <= yEnd; yIndex++) {
            resultRow = resultRow + getColumnByIndex(yIndex, currentRow);
            if (yIndex != yEnd) {
              resultRow = resultRow + ',';
            }
        }
        
        resultPayload = resultPayload + resultRow + '\n';
    }
    return resultPayload;
}
