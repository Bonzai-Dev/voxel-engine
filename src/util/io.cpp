#include <fstream>
#include <string>
#include <iostream>
#include <core/logger.hpp>
#include "io.hpp"

namespace Util {
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

    Logger::logError(
      Logger::Context::CORE,
      "Failed to read file at %s",
      filepath
    );
    return "";
  }
}
