#include "csv_manager.h"

#include <arrow/table.h>
#include <arrow/compute/api.h>
#include <memory>
#include <server_session_controller.h>

#include "tablog.h"

#include <iostream>
#include <algorithm>
#include <functional>
#include <string>
#include <cstring>

void CsvManager::setFile(ttp2::ServerSessionController::File newFile) {
  this->file = newFile;
  logger->log(tablog::DEBUG, "File" + this->file.payload->ToString());
}

std::string CsvManager::getFilePath() {
  return this->file.filePath;
}

int CsvManager::getRowCount() {
  return this->file.payload->num_rows();
}

int CsvManager::getColumnCount() {
  return this->file.payload->num_columns();
}

std::shared_ptr<arrow::ChunkedArray> CsvManager::getColumnByIndex(int index) {
  return this->file.payload->column(index);
}

std::shared_ptr<arrow::Table> CsvManager::getViewport(int xStart, int xEnd, int yStart, int yEnd) {
  yStart = yStart - this->file.start;
  yEnd = yEnd - this->file.start;
  
  int columnCount = this->file.payload->num_columns()-1;
  if (yEnd > columnCount) {
    yEnd = columnCount;
  }

  logger->log(tablog::DEBUG, "yStart " + std::to_string(yStart) + " yEnd " + std::to_string(yEnd));

  int rowCount = this->file.payload->num_rows();
  if (xEnd > rowCount) {
    xEnd = rowCount;
  }

  // Create row filter map
  std::vector<bool> filterVector = {};

  for (int index = 0; index < rowCount; index++) {
    if (index >= xStart && index <= xEnd) {
      filterVector.push_back(true);
    } else {
      filterVector.push_back(false);
    }
  }

  arrow::BooleanBuilder builder;
  arrow::Status status = builder.AppendValues(filterVector);
  std::shared_ptr<arrow::Array> filterArray;
  status = builder.Finish(&filterArray);
  if (!status.ok()) {
    logger->log(tablog::ERROR, "");
  }

  std::vector<std::shared_ptr<arrow::Array>> filterChunks = {filterArray};
  std::shared_ptr<arrow::ChunkedArray> filterChunkedArray = std::make_shared<arrow::ChunkedArray>(filterChunks);

  // Calc Viewport
  std::vector<std::shared_ptr<arrow::Field>> fields;
  std::vector<std::shared_ptr<arrow::ChunkedArray>> columns;
 
  for (int index = yStart; index < yEnd; index++) {
    fields.push_back(this->file.payload->field(index));

    arrow::Datum filterResult = arrow::compute::Filter(this->file.payload->column(index), filterChunkedArray).ValueOrDie();
    std::shared_ptr<arrow::ChunkedArray> column = filterResult.chunked_array();
    columns.push_back(column);
  }
   
  std::shared_ptr<arrow::Schema> schema = arrow::schema(std::move(fields));
  std::shared_ptr<arrow::Table> table = arrow::Table::Make(schema, columns, columns[0]->length());

  return table;
}
