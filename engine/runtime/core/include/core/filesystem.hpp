#pragma once
#include <filesystem>
#include <vector>

namespace Engine::Core::FileSystem {
  bool createDirectory(const std::filesystem::path &path);

  bool exists(const std::filesystem::path &path);

  bool move(const std::filesystem::path &oldPath, const std::filesystem::path &newPath);
  bool copy(const std::filesystem::path &oldPath, const std::filesystem::path &newPath);

  bool remove(const std::filesystem::path &path);
  bool rename(const std::filesystem::path &oldPath, const std::filesystem::path &newPath);

  std::vector<uint8_t> readBytes(const std::filesystem::path& filepath);
}
