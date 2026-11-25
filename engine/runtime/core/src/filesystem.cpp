#include <filesystem>
#include <core/filesystem.hpp>

namespace Engine::Core::FileSystem {
  bool createDirectory(const std::filesystem::path &path) {
    return std::filesystem::create_directory(path);
  }

  bool exists(const std::filesystem::path &path) {
    return std::filesystem::exists(path);
  }

  bool remove(const std::filesystem::path &path) {
    return std::filesystem::remove(path);
  }

  bool rename(const std::filesystem::path &oldPath, const std::filesystem::path &newPath) {
    return move(oldPath, newPath);
  }

  bool move(const std::filesystem::path &oldPath, const std::filesystem::path &newPath) {
    if (FileSystem::exists(newPath))
      return false;

    std::filesystem::rename(oldPath, newPath);
    return true;
  }

  bool copy(const std::filesystem::path &oldPath, const std::filesystem::path &newPath) {
    if (FileSystem::exists(newPath))
      return false;

    std::filesystem::copy(oldPath, newPath);
    return true;
  }
}
