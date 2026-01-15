#include <iostream>
#include <core/logger.hpp>
#include "terrain.hpp"

namespace Game {
  Terrain::Terrain(const Camera &camera): camera(camera) {
    Logger::logInfo(Logger::Context::Game, "Generating terrain with a seed of %d.", seed);

    shader.use();
    shader.updateTexture(Renderer::loadPng("./res/images/blocks.png"));

    for (int x = -RenderDistance / 2; x <= RenderDistance / 2; x++) {
      for (int z = -RenderDistance / 2; z <= RenderDistance / 2; z++) {
        chunks.emplace_back(glm::ivec2(x, z), generateHeightMap(glm::ivec2(x, z)));
      }
    }

    Logger::logInfo(Logger::Context::Game, "Terrain generated successfully.");
  }

  void Terrain::render() const {
    shader.use();
    shader.updateProjectionMatrix(camera.getProjectionMatrix());
    shader.updateViewMatrix(camera.getViewMatrix());
    for (const auto & chunk : chunks) {
      shader.updateModelMatrix(chunk.getModelMatrix());
      chunk.render();
    }
  }

  const Chunk &Terrain::getChunk(const glm::ivec2 &position) {
    return chunks[getChunkIndex(position)];
  }

  // const Blocks::Block &Terrain::getBlock(const glm::ivec3 &position) {
  //
  // }

  std::vector<int> Terrain::generateHeightMap(const glm::ivec2 &chunkPosition) const {
    std::vector<int> map;
    for (size_t blockCount = 0; blockCount < ChunkSize * ChunkSize; blockCount++) {
      const auto x = blockCount % ChunkSize + (chunkPosition.x * ChunkSize);
      const int z = (blockCount / ChunkSize) % ChunkSize + (chunkPosition.y * ChunkSize);
      map.push_back(noise2d(glm::ivec2(x, z), glm::ivec2(0.5, 0.5), 10));
    }

    return map;
  }

  size_t Terrain::getChunkIndex(const glm::ivec2 &position) {
    return (position.x / ChunkSize + position.y / ChunkSize) * ChunkSize;
  }

  int Terrain::generateSeed() {
    srand(time(nullptr));
    static constexpr auto seedRange = static_cast<int>(10e5);
    return rand() % (2 * seedRange + 1) - seedRange;
  }

  int Terrain::noise2d(const glm::ivec2 &position, const glm::ivec2 &scale, int amplitude) const {
    return static_cast<int>(floor(noiseGenerator.eval(position.x * scale.x, position.y * scale.y) * amplitude));
  }
}
