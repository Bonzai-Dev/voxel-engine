#pragma once
#include <array>

namespace Game::Blocks {
  enum class BlockId {
    Air = 0,
    Dirt = 1,
  };

  enum class MeshId {
    None = 0,
    Default = 1,
  };

  // Filepaths must be specified in order
  constexpr inline std::array MeshDataFilepaths = {
    "./res/data/mesh/default.json"
  };

  constexpr inline std::array BlockDataFilepaths = {
    "./res/data/block/dirt.json"
  };
}
