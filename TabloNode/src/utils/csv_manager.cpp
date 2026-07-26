#include "csv_manager.h"

#include <arrow/table.h>
#include <arrow/compute/api.h>
#include <arrow/type.h>
#include <memory>
#include <server_session_controller.h>

#include <tablog.h>

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
  logger->log(tablog::DEBUG, "xStart " + std::to_string(xStart) + " xEnd " + std::to_string(xEnd));
  
  // Slice columns
  std::vector<int> selectColumnIndices(yEnd - yStart + 1);
  std::iota(selectColumnIndices.begin(), selectColumnIndices.end(), yStart);
  std::shared_ptr<arrow::Table> columnSliceTable = *this->file.payload->SelectColumns(selectColumnIndices);

  // Slice rows
  std::shared_ptr<arrow::Table> slicedRowTable = columnSliceTable->Slice(xStart, xEnd);

  // logger->log(tablog::DEBUG, "Viewport content:\n" + slicedRowTable->ToString());
  
  return slicedRowTable;
}

std::shared_ptr<arrow::Table> CsvManager::filter(std::string columnName, std::string regex) {
  std::vector<std::shared_ptr<arrow::Field>> fieldNames = this->file.payload->schema()->fields();
  for (int fieldIndex = 0; fieldIndex < fieldNames.size(); fieldIndex++) {
    if (fieldNames[fieldIndex]->name() == columnName) {
      std::shared_ptr<arrow::ChunkedArray> selectedColumn = this->file.payload->GetColumnByName(columnName);

      arrow::BooleanBuilder builder;
      for (int rowIndex = 0; rowIndex < selectedColumn->length(); rowIndex++) {
        arrow::Status status = builder.Append(applyRegexOnScalar(*selectedColumn->GetScalar(rowIndex), regex));
        if(!status.ok()) {
          logger->log(tablog::CRITICAL, "Unable to append regexResult to booleanBuilder in filter");
        }
      }

      arrow::Result<std::shared_ptr<arrow::Array>> maskResult = builder.Finish();
      if (!maskResult.ok()) {
        logger->log(tablog::CRITICAL, "Unable to convert booleanBuilder to std::shared_ptr<arrow::Array>");
      }
      std::shared_ptr<arrow::Array> maskArray = maskResult.ValueUnsafe();

      arrow::Result<arrow::Datum> filterResult = arrow::compute::Filter(this->file.payload, maskArray);
      if (!filterResult.ok()) {
        logger->log(tablog::CRITICAL, "Unable to apply filter mask on table!");
      }
      arrow::Datum filteredDatum = filterResult.ValueUnsafe();

      logger->log(tablog::DEBUG, "TTP2 Works!: " + columnName + " and " + regex);
      std::shared_ptr<arrow::Table> resultTable = filteredDatum.table();
  
      return std::move(resultTable);
    }
  }
  logger->log(tablog::CRITICAL, "Column name does not exist!: " + columnName);
  return nullptr;
}

bool CsvManager::applyRegexOnScalar(const std::shared_ptr<arrow::Scalar>& scalar, const std::string regex) {
  return true;
}
