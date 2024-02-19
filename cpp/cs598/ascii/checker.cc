#include "checker.h"
#include <arrow/compare.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iostream>
#include <string>
#include <cmath>        // For std::round
#include <iomanip>      // For std::setprecision
#include "rapidcsv.h"
#include <vector>

std::string vectorToString(const std::vector<float>& vec) {
    std::stringstream ss;
    for (size_t i = 0; i < vec.size(); ++i) {
        ss << std::fixed << std::setprecision(2) << vec[i];
        // if (i < vec.size() - 1) {
        //     ss << "\\000"; // Add separator except after the last element
        // }
    }
    return ss.str();
}

std::string floatToString(const float element) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << element;
    return ss.str();
}

std::string intToString(const int element) {
    std::stringstream ss;
    ss << element;
    return ss.str();
}

std::string vectorToString(const std::vector<int>& vec) {
    std::stringstream ss;
    for (size_t i = 0; i < vec.size(); ++i) {
        ss << vec[i];
        // if (i < vec.size() - 1) {
        //     ss << "\000"; // Add separator except after the last element
        // }
    }
    return ss.str();
}


bool containsSubstring(const std::string& str, const std::string& substr) {
    return str.find(substr) != std::string::npos;
}

bool checkEncodingCorrect(const std::string& csvFilePath, const std::string& parquetFilePath){
    std::ifstream parquetFile(parquetFilePath);
    std::string content;
    if (parquetFile) {
        std::stringstream buffer;
        buffer << parquetFile.rdbuf();
        content = buffer.str();
    }
    else{
        std::cerr << "Error opening file: " << parquetFilePath << std::endl;
        return false;
    }

    rapidcsv::Document doc(csvFilePath);

    std::vector<int> col = doc.GetColumn<int>("Integers");
    for (size_t i = 0; i < 20; ++i) {
        if(!containsSubstring(content,intToString(col[i]))){
            return false;
        }
    }

    if(csvFilePath == "data.csv"){
        std::vector<float> col1 = doc.GetColumn<float>("Floats");
        for (size_t i = 0; i < 20; ++i) {
            if(!containsSubstring(content,floatToString(col1[i]))){
                return false;
            }
        }
    }
    parquetFile.close();
    return true;
}

bool checkEfficiency(const int duration_ascii, const int duration_parquet){
    return duration_ascii > duration_parquet;
}

bool checkDecodingCorrect(const arrow::Table& table, const arrow::Table& table1){
    if(table.num_columns() != table1.num_columns()){
        return false;
    }
    for (int i = 0; i < table.num_columns(); ++i) {
      auto actual_col = table1.column(i);
      auto expected_col = table.column(i);

      if(actual_col->length() != expected_col->length()){
        return false;
      }
      auto equal_options = arrow::EqualOptions::Defaults();
      equal_options = equal_options.atol(0.05);

      if(!actual_col->ApproxEquals(*expected_col,equal_options)){
        return false;
      }
    }
    return true;
}

