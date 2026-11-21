#include <engine/runtime/core/filesystem.hpp>
#include <engine/runtime/core/unix/unix_filesystem.hpp>

namespace Engine::Core::FileSystem {
  constexpr bool readBytes(const std::filesystem::path& filepath) {
    return true;
  }
}

namespace Engine::Core {
}
