#include "checker.h"
#include <arrow/csv/api.h>
#include <arrow/io/api.h>
#include <arrow/ipc/api.h>
#include <arrow/pretty_print.h>
#include <arrow/result.h>
#include <arrow/status.h>
#include <arrow/table.h>
#include <parquet/arrow/reader.h>
#include <parquet/arrow/writer.h>
#include <chrono>
#include <iostream>

// Do not change this file!!!

using arrow::Status;
namespace {
int score = 0;
Status RunTest(bool testFloat) {
  const char* data_filename;
  if(!testFloat){
    data_filename = "data_int.csv";
  }else{
    data_filename = "data.csv";
  }
  const char* ascii_filename = "test_ascii";
  
  std::cerr << "* Reading data '" << data_filename << "' from csv" << std::endl;
  auto start = std::chrono::high_resolution_clock::now();
  ARROW_ASSIGN_OR_RAISE(auto input_file, arrow::io::ReadableFile::Open(data_filename));
  auto convertOptions = arrow::csv::ConvertOptions::Defaults();
  std::unordered_map<std::string, std::shared_ptr<arrow::DataType>> column_types;
  column_types["Integers"] = arrow::int64();
  column_types["Floats"] = arrow::float32();
  convertOptions.column_types = column_types;
  ARROW_ASSIGN_OR_RAISE(auto csv_reader, arrow::csv::TableReader::Make(
                                             arrow::io::default_io_context(), input_file,
                                             arrow::csv::ReadOptions::Defaults(),
                                             arrow::csv::ParseOptions::Defaults(),
                                             convertOptions));
  ARROW_ASSIGN_OR_RAISE(auto table, csv_reader->Read());
  auto stop = std::chrono::high_resolution_clock::now();
  auto duration_ascii = std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count();
  std::cout << "Time for reading data from csv: " << duration_ascii << std::endl;

  // std::cerr << "* Print Arrow table:" << std::endl;
  // ARROW_RETURN_NOT_OK(arrow::PrettyPrint(*table, {}, &std::cerr));

  using parquet::ArrowWriterProperties;
  using parquet::WriterProperties;
  using parquet::ParquetVersion;
  using parquet::ParquetDataPageVersion;
  using arrow::Compression;
  using parquet::Encoding;
  std::shared_ptr<WriterProperties> props = WriterProperties::Builder().disable_dictionary()->encoding(Encoding::ASCII)->build();

  std::cerr << "* Writing table into Parquet file using ASCII encoding '" << ascii_filename << "'"
            << std::endl;
  ARROW_ASSIGN_OR_RAISE(auto outfile,
                        arrow::io::FileOutputStream::Open(ascii_filename));
  ARROW_RETURN_NOT_OK(parquet::arrow::WriteTable(
      *table, arrow::default_memory_pool(), outfile, 100000,props));//chunk size is set to 100000(row number of data)
      // so that all data are in the same row group

  //check1: the correctness of encoder
  if(checkEncodingCorrect(data_filename,ascii_filename)){
    score += 25;
    std::cout<<"Encoding correctness check succeed"<<std::endl;
  }else{
    std::cout<<"Encoding correctness check failed"<<std::endl;
  }
  


  std::cerr << "* Reading table from Parquet file using ASCII decoding '" << ascii_filename << "'"
          << std::endl;
  start = std::chrono::high_resolution_clock::now();
  arrow::MemoryPool* pool = arrow::default_memory_pool();
  std::shared_ptr<arrow::io::RandomAccessFile> input;
  ARROW_ASSIGN_OR_RAISE(input, arrow::io::ReadableFile::Open(ascii_filename));
  std::unique_ptr<parquet::arrow::FileReader> arrow_reader;
  ARROW_RETURN_NOT_OK(parquet::arrow::OpenFile(input, pool, &arrow_reader));
  std::shared_ptr<arrow::Table> table1;
  ARROW_RETURN_NOT_OK(arrow_reader->ReadTable(&table1));
  stop = std::chrono::high_resolution_clock::now();
  auto duration_parquet = std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count();
  std::cout << "Time for reading data from parquet: " << duration_parquet << std::endl; 

  // std::cerr << "* Print Arrow Table Read back:" << std::endl;
  // ARROW_RETURN_NOT_OK(arrow::PrettyPrint(*table1, {}, &std::cerr));

  // check2: efficiency of decoder
  // the time of reading parquet should be shorter than reading from csv because parquet is column storage format.
  if(checkEfficiency(duration_ascii, duration_parquet)){
    score += 5;
    std::cout<<"Decoding efficiency check succeed"<<std::endl;
  }else{
    std::cout<<"Decoding efficiency check failed"<<std::endl;
  }

  //check3: correctness of decoder
  if(checkDecodingCorrect(*table1, *table)){
    score += 20;
    std::cout<<"decoding correctness check succeed"<<std::endl;
  }else{
    std::cout<<"decoding correctness check failed"<<std::endl;
  }

  return Status::OK();
}

}

int main(int argc, char** argv) {
  std::cout << "============= test integer encoder/decoder=========== " << std::endl;
  Status st1 = RunTest(false);
  std::cout << "============= test float encoder/decoder=========== " << std::endl;
  Status st2 = RunTest(true);
  std::cout << "final score: " << score << std::endl;
  if (!st1.ok()) {
    std::cerr << st1 << std::endl;
    return 1;
  }
  if (!st2.ok()) {
    std::cerr << st2 << std::endl;
    return 1;
  }
  return 0;
}
