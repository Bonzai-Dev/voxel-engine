#pragma once
#include <simdjson.h>
#include <core/logger.hpp>

namespace Util::File {
  std::string readTextFile(std::string_view filepath);
}

namespace Util::Json {
  using namespace simdjson;

  dom::element parseJson(std::string_view filepath);

  template <typename T>
  T getData(std::string_view name, const dom::element &document) {
    T value;
    auto error = document[name.data()].get(value);
    if (error) {
      Logger::logError(Logger::Context::Core, "Failed to get \"%s\", %s", name, simdjson::error_message(error));
      return T{};
    }
    return value;
  }

  template <typename T>
  T getIndexValue(size_t index, const dom::array &array) {
    T value;
    auto error = array.at(index).get(value);
    if (error) {
      Logger::logError(Logger::Context::Core, "Failed to get index %u: %s", index, simdjson::error_message(error));
      return T{};
    }
    return value;
  }
}
