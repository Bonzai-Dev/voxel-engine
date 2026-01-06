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

      const glm::vec3 &GetPosition() const { return m_position; }

      const BlockId &GetBlockId() const { return m_blockId; }

      const BlockMeshData &GetMesh() const { return m_blockMeshData[m_blockId]; }

    private:
      static inline std::unordered_map<BlockId, BlockMeshData> m_blockMeshData;
      static inline std::unordered_map<MeshId, Renderer::MeshData> m_meshData;
      static inline std::unordered_map<BlockId, BlockData> m_blockData;

      const BlockId m_blockId;
      MeshId m_meshId = MeshId::None;
      glm::vec3 m_position;

      void SerializeBlockData() const;

      static void LoadMesh(MeshId meshId);

      void LoadBlockMesh() const;

      static glm::vec2 GenerateUv(const glm::ivec2 &spritesheetTile, const glm::vec3 &vertexPosition, Face face);
  };
}
