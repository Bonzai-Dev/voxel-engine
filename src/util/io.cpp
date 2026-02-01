#include <fstream>
#include <string>
#include <iostream>
#include <simdjson.h>
#include <core/logger.hpp>
#include "io.hpp"

using namespace Logger;

namespace Util::File {
  std::string readTextFile(std::string_view filepath) {
    // https://stackoverflow.com/questions/195323/what-is-the-most-elegant-way-to-read-a-text-file-with-c
    std::ifstream in((filepath.data()));
    std::string contents((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

    if (!in) {
      logError(
        Context::Core,
        "Failed to read file at \"%s.\"",
        filepath.data()
      );
      return {};
    }

    return contents;
  }
}

namespace Util::Json {
  using namespace simdjson;

  dom::element parseJson(std::string_view filepath) {
    logInfo(Context::Core, "Reading JSON file at \"%s\".", filepath.data());
    static dom::parser parser;
    dom::element document;
    auto error = parser.load(filepath).get(document);
    if (error) {
      logError(Context::Core, "Failed to parse JSON file at %s: %s.", filepath.data(), error_message(error));
      return {};
    }
    return document;
  }
}
