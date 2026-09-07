#include "csv_manager.h"

#include <arrow/table.h>

#include <lexer.h>

#include <memory>

#include <server_session_controller.h>
#include <tablog.h>

#include <string>

CsvManager::CsvManager() {
  interpreter.setOpenFile([this](std::string filePath) {
    return this->executionEndpoint.setSelectedFile(filePath);
  });
  
  interpreter.setGetWhere([this](std::string operatorName, std::string columnName,
                                               std::string compareValue, std::shared_ptr<arrow::Table> table) {
    return this->executionEndpoint.getWhere(operatorName, columnName, compareValue, table);
  });
  
  interpreter.setSelectColumns([this](std::vector<std::string> columnNames,
                                                     std::shared_ptr<arrow::Table> table) {
    return this->executionEndpoint.selectColumns(columnNames, table);
  });
  
  interpreter.setGetDistinct([this](std::shared_ptr<arrow::Table> table) {
    return this->executionEndpoint.getDistinct(table);
  });
  
  interpreter.setGetCount([this](std::shared_ptr<arrow::Table> table) {
    return this->executionEndpoint.getCount(table);
  });
  
  interpreter.setGetMin([this](std::shared_ptr<arrow::Table> table) {
    return this->executionEndpoint.getMin(table);
  });
  
  interpreter.setGetMax([this](std::shared_ptr<arrow::Table> table) {
    return this->executionEndpoint.getMax(table);
  });
  
  interpreter.setGetSum([this](std::shared_ptr<arrow::Table> table) {
    return this->executionEndpoint.getSum(table);
  });
  
  interpreter.setGetAvg([this](std::shared_ptr<arrow::Table> table) {
    return this->executionEndpoint.getAvg(table);
  });
  
  interpreter.setGetRenamedTable([this](std::string originalColumnName,
                                                       std::string newColumnName,
                                                       std::shared_ptr<arrow::Table> table) {
    return this->executionEndpoint.getRenamedTable(originalColumnName, newColumnName, table);
  });
}

void CsvManager::setFile(ttp2::ServerSessionController::File newFile) {
  this->executionEndpoint.registerFile(newFile);
  logger->log(tablog::DEBUG, "File" + this->executionEndpoint.getSelectedFile()->payload->ToString());
}

std::string CsvManager::getFilePath() {
  return this->executionEndpoint.getSelectedFile()->filePath;
}

int CsvManager::getRowCount() {
  return this->executionEndpoint.getSelectedFile()->payload->num_rows();
}

int CsvManager::getColumnCount() {
  return this->executionEndpoint.getSelectedFile()->payload->num_columns();
}

std::shared_ptr<arrow::ChunkedArray> CsvManager::getColumnByIndex(int index) {
  return this->executionEndpoint.getSelectedFile()->payload->column(index);
}

std::shared_ptr<arrow::Table> CsvManager::getViewport(int xStart, int xEnd, int yStart, int yEnd) {  
  int columnCount = this->executionEndpoint.getSelectedFile()->payload->num_columns()-1;
  if (yEnd > columnCount)
    yEnd = columnCount;

  if (yStart < 0)
    yStart = 0;

  logger->log(tablog::DEBUG, "yStart " + std::to_string(yStart) + " yEnd " + std::to_string(yEnd));

  int rowStartIndex = this->executionEndpoint.getSelectedFile()->start;
  xStart = xStart - rowStartIndex;
  if (xStart < 0)
    xStart = 0;
  
  xEnd = xEnd - rowStartIndex;
  if (xEnd > this->executionEndpoint.getSelectedFile()->end)
    xEnd = this->executionEndpoint.getSelectedFile()->end;
  logger->log(tablog::DEBUG, "xStart " + std::to_string(xStart) + " xEnd " + std::to_string(xEnd));
  
  // Slice columns
  std::vector<int> selectColumnIndices(yEnd - yStart + 1);
  std::iota(selectColumnIndices.begin(), selectColumnIndices.end(), yStart);
  std::shared_ptr<arrow::Table> columnSliceTable = *this->executionEndpoint.getSelectedFile()->payload->SelectColumns(selectColumnIndices);

  // Slice rows
  std::shared_ptr<arrow::Table> slicedRowTable = columnSliceTable->Slice(xStart, xEnd);

  // logger->log(tablog::DEBUG, "Viewport content:\n" + slicedRowTable->ToString());
  
  return slicedRowTable;
}

std::shared_ptr<arrow::Table> CsvManager::executeQuery(const std::string& query) {
  tql::Lexer lexer;
  lexer.tokenize(query);

  tql::Parser::Expression expression = this->parser.parse(lexer);
  
  return this->interpreter.interpret(expression);
}
