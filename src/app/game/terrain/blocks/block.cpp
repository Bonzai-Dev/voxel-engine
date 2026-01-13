#include <simdjson.h>
#include <filesystem>
#include <util/io.hpp>
#include <glm/detail/func_geometric.inl>
#include "block_data.hpp"
#include "block.hpp"

using namespace Logger;

namespace Game::Blocks {
  Block::Block(const glm::vec3 &position, BlockId blockId) : blockId(blockId), position(position) {
    if (!blockData.contains(blockId))
      serializeBlockData();

    meshId = blockData[blockId].meshId;

    if (!meshData.contains(meshId))
      loadMesh(meshId);

    if (!blockMeshData.contains(blockId))
      loadBlockMesh();
  }

  void Block::loadBlockMesh() const {
    if (blockId == BlockId::Air)
      return;

    const auto &mesh = meshData[meshId];
    auto &blockMesh = blockMeshData[blockId];
    const auto &data = blockData[blockId];

    for (unsigned int vertexIndex = 0; vertexIndex < meshData[meshId].vertexData.size(); vertexIndex++) {
      const auto &vertex = meshData[meshId].vertexData[vertexIndex];
      const unsigned int faceIndex = vertexIndex / 4;
      const Face face = static_cast<Face>(faceIndex);

      const glm::ivec2 tilePosition = data.spritesheetTiles[faceIndex];
      const glm::vec2 uv = generateUv(tilePosition, vertex.position, face);
      blockMesh.vertexData.emplace_back(BlockVertex{vertex.position, uv});
    }

    blockMesh.indices = mesh.indices;
  }

  void Block::serializeBlockData() const {
    if (blockId == BlockId::Air)
      return;

    using namespace Util::Json;
    // Subtracting by 1 to account for Air block at index 0
    const auto index = static_cast<unsigned int>(blockId) - 1;
    const auto data = parseJson(BlockDataFilepaths[index]);
    const auto meshIndex = getData<std::int64_t>("meshId", data);
    const auto meshId = static_cast<MeshId>(meshIndex);
    const auto spritesheetTiles = getData<dom::object>("spritesheetTiles", data);
    const std::array spritesheetTileArray = {
      getData<dom::array>("front", spritesheetTiles),
      getData<dom::array>("back", spritesheetTiles),
      getData<dom::array>("left", spritesheetTiles),
      getData<dom::array>("right", spritesheetTiles),
      getData<dom::array>("top", spritesheetTiles),
      getData<dom::array>("bottom", spritesheetTiles)
    };

    auto &serializedData = blockData[blockId];
    serializedData.meshId = meshId;
    serializedData.blockId = blockId;
    for (unsigned int faceIndex = 0; faceIndex < 6; faceIndex++) {
      serializedData.spritesheetTiles.emplace_back(
        getIndexValue<std::int64_t>(0, spritesheetTileArray[faceIndex]),
        getIndexValue<std::int64_t>(1, spritesheetTileArray[faceIndex])
      );
    }
  }

  void Block::loadMesh(MeshId meshId) {
    if (meshId == MeshId::None)
      return;

    using namespace simdjson;
    using namespace Util::Json;

    const auto meshIndex = static_cast<size_t>(meshId);
    const auto data = parseJson(MeshDataFilepaths[meshIndex - 1]);
    const auto facesData = getData<dom::object>("faces", data);
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

      Renderer::MeshData &mesh = meshData[meshId];
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

  glm::vec2 Block::generateUv(const glm::ivec2 &spritesheetTile, const glm::vec3 &vertexPosition, Face face) {
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
