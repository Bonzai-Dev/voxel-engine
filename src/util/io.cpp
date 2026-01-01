#include <fstream>
#include <string>
#include <iostream>
#include <simdjson.h>
#include <core/logger.hpp>
#include "io.hpp"

using namespace Logger;

namespace Util::File {
  const char *readTextFile(const char *filepath) {
    // https://stackoverflow.com/questions/195323/what-is-the-most-elegant-way-to-read-a-text-file-with-c
    std::ifstream file(filepath, std::ios::in | std::ios::binary | std::ios::ate);
    if (file.is_open()) {
      file.seekg(0, std::ios::end);
      const int size = file.tellg();
      const auto contents = new char [size];
      contents[size] = '\0';

      file.seekg(0, std::ios::beg);
      file.read(contents, size);
      file.close();
      return contents;
    }

    logError(
      Context::Core,
      "Failed to read file at %s",
      filepath
    );
    return {};
  }
}

namespace Util::Json {
  using namespace simdjson;
  dom::element parseJson(const char *filepath) {
    static dom::parser parser;
    dom::element document;
    auto error = parser.load(filepath).get(document);
    if (error) {
      logError(Context::Core, "Failed to parse JSON file at %s: %s", filepath, error_message(error));
      return {};
    }
    return document;
  }
}