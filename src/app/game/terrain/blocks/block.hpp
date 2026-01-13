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

      const MeshId &getMeshId() const { return meshId; }

      const BlockMeshData &getMesh() const { return blockMeshData[blockId]; }

    private:
      static inline std::unordered_map<BlockId, BlockMeshData> blockMeshData;
      static inline std::unordered_map<MeshId, Renderer::MeshData> meshData;
      static inline std::unordered_map<BlockId, BlockData> blockData;

      const BlockId blockId;
      MeshId meshId = MeshId::None;
      glm::vec3 position;

      void serializeBlockData() const;

      static void loadMesh(MeshId meshId);

      void loadBlockMesh() const;

      static glm::vec2 generateUv(const glm::ivec2 &spritesheetTile, const glm::vec3 &vertexPosition, Face face);
  };
}
