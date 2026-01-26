#pragma once
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <core/renderer/renderer.hpp>
#include "block_data.hpp"

namespace Game::Blocks {
  enum class Face {
    Front,
    Back,
    Left,
    Right,
    Top,
    Bottom
  };

  struct BlockVertex : Renderer::MeshVertex {
    glm::vec2 uv;
  };

  using BlockVertexData = std::vector<BlockVertex>;

  struct BlockMeshData {
    BlockVertexData vertexData;
    Renderer::Indices indices;
  };

  struct BlockData {
    BlockId blockId;
    MeshId meshId;
    std::vector<glm::ivec2> spritesheetTiles;
  };

  class Block {
    public:
      Block(const glm::vec3 &position, BlockId blockId);

      const glm::vec3 &getPosition() const { return position; }

      const BlockId &getBlockId() const { return blockId; }

      MeshId getMeshId() const { return meshId; }

    private:
      const BlockId blockId;
      MeshId meshId = MeshId::None;
      glm::vec3 position;
  };
}
