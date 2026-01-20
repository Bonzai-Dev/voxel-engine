#pragma once
#include <array>

namespace Game::Blocks {
  enum class BlockId {
    Air = 0,
    Bedrock = 1,
    Grass = 2,
    Dirt = 3,
    Stone = 4,
  };

  enum class MeshId {
    None = 0,
    Default = 1,
  };

  // Filepaths must be specified in order to the id
  constexpr inline std::array MeshDataFilepaths = {
    "./res/data/mesh/default.json"
  };

  constexpr inline std::array BlockDataFilepaths = {
    "./res/data/block/bedrock.json",
    "./res/data/block/grass.json",
    "./res/data/block/dirt.json",
    "./res/data/block/stone.json"
  };
}
