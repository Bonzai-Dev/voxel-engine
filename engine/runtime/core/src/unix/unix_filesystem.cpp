#include <fstream>
#include <vector>
#include <engine/runtime/core/filesystem.hpp>
#include <engine/runtime/core/logging.hpp>
#include <engine/runtime/core/assert.hpp>
#include <engine/runtime/core/unix/unix_filesystem.hpp>

namespace Engine::Core::FileSystem {
  std::string readBytes(const std::filesystem::path& filepath) {
    std::vector<char> buffer;

    std::ifstream stream(filepath, std::ios::binary | std::ios::ate);
    ENGINE_ASSERT(stream, "Failed to open file: {}", filepath.c_str());

    auto end = stream.tellg();
    stream.seekg(0, std::ios::beg);
    auto size = end - stream.tellg();

    buffer.resize((uint32_t)size);
    stream.read(buffer.data(), buffer.size());
    stream.close();

    return std::string(buffer.begin(), buffer.end());
  }
}

namespace Engine::Core {
}
