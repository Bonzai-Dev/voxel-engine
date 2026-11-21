#pragma once
#include <filesystem>

namespace Engine::Core::FileSystem {
  enum class FileError: uint8_t {
    NotFound,
    AccessDenied,
    AlreadyExists,
    InvalidArgument,
    IoFailure,
    Unknown,
  };

  constexpr bool createDirectory(const std::filesystem::path &path);

  constexpr bool exists(const std::filesystem::path &path);

  constexpr bool move(const std::filesystem::path &oldPath, const std::filesystem::path &newPath);
  constexpr bool copy(const std::filesystem::path &oldPath, const std::filesystem::path &newPath);

  constexpr bool remove(const std::filesystem::path &path);
  constexpr bool rename(const std::filesystem::path &oldPath, const std::filesystem::path &newPath);

  constexpr bool readBytes(const std::filesystem::path& filepath);
}
