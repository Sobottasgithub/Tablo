#include "csv_manager.h"

#include <arrow/table.h>
#include <memory>
#include <server_session_controller.h>

#include <iostream>
#include <algorithm>
#include <functional>
#include <string>
#include <cstring>

void CsvManager::setFile(ttp2::ServerSessionController::File newFile) {
  this->file = newFile;
  std::wcout << "FILE: " << this->file.payload->ToString().c_str() << std::endl;
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

std::string CsvManager::getRowByIndex(int index) {
  // TODO
  // std::shared_ptr<arrow::Scalar> scalar;
  // chunked_array->GetScalar(4).Value(&scalar).ok() && scalar->is_valid
  // std::static_pointer_cast<arrow::Int32Array>
  return "";
}

std::shared_ptr<arrow::ChunkedArray> CsvManager::getColumnByIndex(int index) {
  return this->file.payload->column(index);
}

std::shared_ptr<arrow::Table> CsvManager::getViewport(int xStart, int xEnd, int yStart, int yEnd) {
  // TODO: use xStart and xEnd
  int columnCount = this->file.payload->num_columns();
  if (yEnd > columnCount) {
    yEnd = columnCount;
  }

  std::vector<std::shared_ptr<arrow::Field>> fields;
  std::vector<std::shared_ptr<arrow::ChunkedArray>> columns;
 
  for (int index = yStart; index < yEnd; index++) {
    fields.push_back(this->file.payload->field(index));
    columns.push_back(this->file.payload->column(index));
  }
   
  std::shared_ptr<arrow::Schema> schema = arrow::schema(std::move(fields));
  std::shared_ptr<arrow::Table> table = arrow::Table::Make(schema, columns, columns[0]->length());

  return table;
}
