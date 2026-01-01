#pragma once
#include <unordered_map>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace Game {
  enum class Face {
    Front,
    Back,
    Left,
    Right,
    Top,
    Bottom
  };

  enum class MeshId {
    None,
    Default,
    Liquid
  };

  enum class BlockId {
    Air,
    Dirt,
    Grass,
    Stone,
    Water
  };

  struct MeshVertex {
    glm::vec3 position;
    glm::vec2 uv;
  };

  class Block {
    public:
      Block(const glm::vec3 &position, BlockId id);

      static const std::vector<MeshVertex> &GetVertexData(MeshId meshType) { return m_vertexData[meshType]; }

      const MeshId &GetMeshId() const { return m_meshId; }

      const BlockId &GetBlockId() const { return m_blockId; }

      const glm::vec3 &GetPosition() const { return m_position; }

    protected:
      glm::vec3 m_position;
      BlockId m_blockId = BlockId::Air;
      MeshId m_meshId = MeshId::None;

      static inline std::unordered_map<MeshId, std::vector<MeshVertex>> m_vertexData;

      void GenerateVertexData() const;

      void LoadBlockData(const char *dataFilepath);

      std::array<glm::ivec2, 6> m_textureData{};
  };
}
