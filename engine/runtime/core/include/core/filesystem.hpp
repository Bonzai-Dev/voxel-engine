#pragma once
#include <filesystem>
#include <vector>

namespace Engine::Core::FileSystem {
  constexpr bool createDirectory(const std::filesystem::path &path);

  constexpr bool exists(const std::filesystem::path &path);

  constexpr bool move(const std::filesystem::path &oldPath, const std::filesystem::path &newPath);
  constexpr bool copy(const std::filesystem::path &oldPath, const std::filesystem::path &newPath);

  constexpr bool remove(const std::filesystem::path &path);
  constexpr bool rename(const std::filesystem::path &oldPath, const std::filesystem::path &newPath);

  std::vector<uint8_t> readBytes(const std::filesystem::path& filepath);
}
