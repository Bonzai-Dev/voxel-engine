#include <vector>
#include <iosfwd>
#include <fstream>
#include <core/logger.hpp>
#include "file.hpp"

namespace Core {
  std::vector<char> readBinaryFile(std::string_view filePath) {
    std::ifstream file(filePath.data(), std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
      LOG_CORE_ERROR("Failed reading binary file at {}", filePath);
      return {};
    }

    std::vector<char> buffer(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
    file.close();

    return buffer;
  }
}
