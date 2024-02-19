// DO NOT CHANGE THIS FILE
#ifndef CHECKER_HPP
#define CHECKER_HPP

#include <string>
#include <arrow/table.h>

bool checkEncodingCorrect(const std::string& csvFilePath, const std::string& parquetFilePath);
bool checkEfficiency(const int duration_ascii, const int duration_parquet);
bool checkDecodingCorrect(const arrow::Table& table, const arrow::Table& table1);

#endif 
