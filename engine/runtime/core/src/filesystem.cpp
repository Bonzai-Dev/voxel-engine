#include <filesystem>
#include <core/filesystem.hpp>

namespace Engine::Core::FileSystem {
  constexpr bool createDirectory(const std::filesystem::path &path) {
    return std::filesystem::create_directory(path);
  }

  constexpr bool exists(const std::filesystem::path &path) {
    return std::filesystem::exists(path);
  }

  constexpr bool remove(const std::filesystem::path &path) {
    return std::filesystem::remove(path);
  }

  constexpr bool rename(const std::filesystem::path &oldPath, const std::filesystem::path &newPath) {
    return move(oldPath, newPath);
  }

  constexpr bool move(const std::filesystem::path &oldPath, const std::filesystem::path &newPath) {
    if (FileSystem::exists(newPath))
      return false;

    std::filesystem::rename(oldPath, newPath);
    return true;
  }

  constexpr bool copy(const std::filesystem::path &oldPath, const std::filesystem::path &newPath) {
    if (FileSystem::exists(newPath))
      return false;

    std::filesystem::copy(oldPath, newPath);
    return true;
  }
}
