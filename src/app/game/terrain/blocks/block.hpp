#pragma once
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <core/renderer/renderer.hpp>
#include "block_data.hpp"
#include "block_manager.hpp"

namespace Game::Blocks {
  enum class Face {
    Front,
    Back,
    Left,
    Right,
    Top,
    Bottom
  };

  class Block {
    public:
      Block(const glm::vec3 &position, BlockId blockId);

      const glm::vec3 &getPosition() const { return position; }

      const BlockId &getBlockId() const { return blockId; }

      const MeshId &getMeshId() const { return meshId; }

      const BlockMeshData &getMesh() const { return BlockManager::getBlockMeshData(blockId); }

    private:
      const BlockId blockId;
      MeshId meshId = MeshId::None;
      glm::vec3 position;
  };
}
