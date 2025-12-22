#include <fstream>
#include <string>
#include <core/logger.hpp>
#include "io.hpp"

#include <iostream>

const char *getFileContents(const char *filepath) {
  // https://stackoverflow.com/questions/195323/what-is-the-most-elegant-way-to-read-a-text-file-with-c
  std::ifstream file(filepath, std::ios::in | std::ios::binary | std::ios::ate);
  if (file.is_open()) {
    file.seekg(0, std::ios::end);
    const int size = file.tellg();
    const auto contents = new char [size];
    file.seekg(0, std::ios::beg);
    file.read(contents, size);
    file.close();
    return contents;
  }

  logger::logError(
    logger::Context::CORE,
    "Failed to read file at path: %s",
    filepath
  );
  return "";
}
