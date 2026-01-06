#include <simdjson.h>
#include <filesystem>
#include <util/io.hpp>
#include <glm/detail/func_geometric.inl>
#include "block_data.hpp"
#include "block.hpp"

using namespace Logger;

namespace Game::Blocks {
  Block::Block(const glm::vec3 &position, BlockId blockId) : m_blockId(blockId), m_position(position) {
    if (!m_blockData.contains(m_blockId))
      SerializeBlockData();

    m_meshId = m_blockData[m_blockId].meshId;

    if (!m_meshData.contains(m_meshId))
      LoadMesh(m_meshId);

    if (!m_blockMeshData.contains(m_blockId))
      LoadBlockMesh();
  }

  void Block::LoadBlockMesh() const {
    if (m_blockId == BlockId::Air)
      return;

    const auto &mesh = m_meshData[m_meshId];
    auto &blockMesh = m_blockMeshData[m_blockId];
    const auto &blockData = m_blockData[m_blockId];

    for (unsigned int vertexIndex = 0; vertexIndex < m_meshData[m_meshId].vertexData.size(); vertexIndex++) {
      const auto &vertex = m_meshData[m_meshId].vertexData[vertexIndex];
      const unsigned int faceIndex = vertexIndex / 4;
      const Face face = static_cast<Face>(faceIndex);

      const glm::ivec2 tilePosition = blockData.spritesheetTiles[faceIndex];
      const glm::vec2 uv = GenerateUv(tilePosition, vertex.position, face);
      blockMesh.vertexData.emplace_back(BlockVertex{vertex.position, uv});
    }

    blockMesh.indices = mesh.indices;
  }

  void Block::SerializeBlockData() const {
    if (m_blockId == BlockId::Air)
      return;

    using namespace Util::Json;
    // Subtracting by 1 to account for Air block at index 0
    const auto index = static_cast<unsigned int>(m_blockId) - 1;
    const auto blockData = parseJson(BlockDataFilepaths[index]);
    const auto meshIndex = getData<std::int64_t>("meshId", blockData);
    const auto meshId = static_cast<MeshId>(meshIndex);
    const auto spritesheetTiles = getData<dom::object>("spritesheetTiles", blockData);
    const std::array spritesheetTileArray = {
      getData<dom::array>("front", spritesheetTiles),
      getData<dom::array>("back", spritesheetTiles),
      getData<dom::array>("left", spritesheetTiles),
      getData<dom::array>("right", spritesheetTiles),
      getData<dom::array>("top", spritesheetTiles),
      getData<dom::array>("bottom", spritesheetTiles)
    };

    auto &serializedData = m_blockData[m_blockId];
    serializedData.meshId = meshId;
    serializedData.blockId = m_blockId;
    for (unsigned int faceIndex = 0; faceIndex < 6; faceIndex++) {
      serializedData.spritesheetTiles.emplace_back(
        getIndexValue<std::int64_t>(0, spritesheetTileArray[faceIndex]),
        getIndexValue<std::int64_t>(1, spritesheetTileArray[faceIndex])
      );
    }
  }

  void Block::LoadMesh(MeshId meshId) {
    if (meshId == MeshId::None)
      return;

    using namespace simdjson;
    using namespace Util::Json;

    const auto meshIndex = static_cast<size_t>(meshId);
    const auto meshData = parseJson(MeshDataFilepaths[meshIndex - 1]);
    const auto facesData = getData<dom::object>("faces", meshData);
    const std::array faces = {
      getData<dom::object>("front", facesData),
      getData<dom::object>("back", facesData),
      getData<dom::object>("left", facesData),
      getData<dom::object>("right", facesData),
      getData<dom::object>("top", facesData),
      getData<dom::object>("bottom", facesData)
    };

    for (unsigned int faceIndex = 0; faceIndex < faces.size(); faceIndex++) {
      const std::array vertexData = {
        getData<dom::array>("bottomLeft", faces[faceIndex]),
        getData<dom::array>("bottomRight", faces[faceIndex]),
        getData<dom::array>("topRight", faces[faceIndex]),
        getData<dom::array>("topLeft", faces[faceIndex]),
      };

      Renderer::MeshData &mesh = m_meshData[meshId];
      for (auto &vertex : vertexData) {
        mesh.vertexData.emplace_back(glm::vec3(
          getIndexValue<double>(0, vertex),
          getIndexValue<double>(1, vertex),
          getIndexValue<double>(2, vertex)
        ));
      }

      const unsigned int index = faceIndex * 4;
      // Top triangle
      mesh.indices.push_back(index + 2);
      mesh.indices.push_back(index + 3);
      mesh.indices.push_back(index + 1);

      // Bottom triangle
      mesh.indices.push_back(index + 3);
      mesh.indices.push_back(index);
      mesh.indices.push_back(index + 1);
    }
  }

  glm::vec2 Block::GenerateUv(const glm::ivec2 &spritesheetTile, const glm::vec3 &vertexPosition, Face face) {
    static constexpr int tilesPerRow = 16;
    static constexpr float tileUvSize = 1.0f / tilesPerRow;

    glm::vec2 uvCoordinates(0, 0);
    if (face == Face::Front || face == Face::Back)
      uvCoordinates = glm::vec2(vertexPosition.x, vertexPosition.y);
    else if (face == Face::Left || face == Face::Right)
      uvCoordinates = glm::vec2(vertexPosition.z, vertexPosition.y);
    else if (face == Face::Top || face == Face::Bottom)
      uvCoordinates = glm::vec2(vertexPosition.x, vertexPosition.z);

    return glm::vec2(spritesheetTile) * tileUvSize + uvCoordinates * tileUvSize;
  }
}
