#pragma once
#include <simdjson.h>
#include <core/logger.hpp>

namespace Util::File {
  const char *readTextFile(const char *filepath);
}

namespace Util::Json {
  using namespace simdjson;

  dom::element parseJson(const char *filepath);

  template <typename T>
  T getData(const char *name, const dom::element &document) {
    T value;
    auto error = document[name].get(value);
    if (error) {
      Logger::logError(Logger::Context::Core, "Failed to get %s: %s", name, simdjson::error_message(error));
      return T{};
    }
    return value;
  }

  template <typename T>
  T getIndexValue(size_t index, const dom::array &array) {
    T value;
    auto error = array.at(index).get(value);
    if (error) {
      Logger::logError(Logger::Context::Core, "Failed to get index %zu: %s", index, simdjson::error_message(error));
      return T{};
    }
    return value;
  }
}
