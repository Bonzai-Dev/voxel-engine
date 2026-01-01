#include <simdjson.h>
#include <core/logger.hpp>
#include <util/io.hpp>
#include "block.hpp"

using namespace Logger;

namespace Game {
  Block::Block(const glm::vec3 &position, BlockId id) : m_position(position), m_blockId(id) {
  }

  void Block::GenerateVertexData() const {
    if (m_vertexData.contains(m_meshId) || m_meshId == MeshId::None) return;

    std::vector<MeshVertex> &vertexData = m_vertexData[m_meshId];
    for (unsigned int faceIndex = 0; faceIndex < 6; faceIndex++) {
      // const glm::vec3 vertexPosition = cube[faceIndex];
      vertexData.push_back({
        .position = glm::vec3(
          // defaultBlockVertices[cubeIndices[faceIndex * 6]],
          // defaultBlockVertices[cubeIndices[faceIndex * 6] + 1],
          // defaultBlockVertices[cubeIndices[faceIndex * 6] + 2]
        ),
        .uv{}
      });
    }
  }

  void Block::LoadBlockData(const char *dataFilepath) {
    using namespace simdjson;
    using namespace Util::Json;
    const auto data = parseJson(dataFilepath);
    const auto blockId = getData<std::uint64_t>("blockId", data);
    const auto meshId = getData<std::uint64_t>("meshId", data);
    const auto textureCoordinates = getData<dom::object>("textureCoordinates", data);

    const auto frontFace = getData<dom::array>("front", textureCoordinates);
    const auto backFace = getData<dom::array>("back", textureCoordinates);
    const auto leftFace = getData<dom::array>("left", textureCoordinates);
    const auto rightFace = getData<dom::array>("right", textureCoordinates);
    const auto topFace = getData<dom::array>("top", textureCoordinates);
    const auto bottomFace = getData<dom::array>("bottom", textureCoordinates);

    m_textureData = {
      glm::ivec2(getIndexValue<std::uint64_t>(0, frontFace), getIndexValue<std::uint64_t>(1, frontFace)),
      glm::ivec2(getIndexValue<std::uint64_t>(0, backFace), getIndexValue<std::uint64_t>(1, backFace)),
      glm::ivec2(getIndexValue<std::uint64_t>(0, leftFace), getIndexValue<std::uint64_t>(1, leftFace)),
      glm::ivec2(getIndexValue<std::uint64_t>(0, rightFace), getIndexValue<std::uint64_t>(1, rightFace)),
      glm::ivec2(getIndexValue<std::uint64_t>(0, topFace), getIndexValue<std::uint64_t>(1, topFace)),
      glm::ivec2(getIndexValue<std::uint64_t>(0, bottomFace), getIndexValue<std::uint64_t>(1, bottomFace))
    };
    m_meshId = static_cast<MeshId>(meshId);
    m_blockId = static_cast<BlockId>(blockId);
  }
}
