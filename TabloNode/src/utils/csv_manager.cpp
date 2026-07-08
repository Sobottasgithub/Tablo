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
  int columnCount = this->file.payload->num_columns()-1;
  if (yEnd > columnCount)
    yEnd = columnCount;

  if (yStart < 0)
    yStart = 0;

  logger->log(tablog::DEBUG, "yStart " + std::to_string(yStart) + " yEnd " + std::to_string(yEnd));

  int rowStartIndex = this->file.start;
  xStart = xStart - rowStartIndex;
  if (xStart < 0)
    xStart = 0;
  xEnd = xEnd - rowStartIndex;
  if (xEnd > this->file.end)
    xEnd = this->file.end;
  
  // Slice columns
  std::vector<std::shared_ptr<arrow::Field>> fields;
  std::vector<std::shared_ptr<arrow::ChunkedArray>> columns;
  
  for (int index = yStart; index <= yEnd; index++) {
    fields.push_back(this->file.payload->field(index));
    columns.push_back(this->file.payload->column(index));
  }
   
  std::shared_ptr<arrow::Schema> schema = arrow::schema(std::move(fields));
  std::shared_ptr<arrow::Table> columnSliceTable = arrow::Table::Make(schema, columns, columns[0]->length());

  // Slice rows
  std::shared_ptr<arrow::Table> slicedRowTable = columnSliceTable->Slice(xStart, xEnd);

  logger->log(tablog::DEBUG, "Viewport content:\n" + slicedRowTable->ToString());
  
  return slicedRowTable;
}
