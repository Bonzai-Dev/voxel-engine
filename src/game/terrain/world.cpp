#include <thread>
#include <map>
#include <core/logger.hpp>
#include "../camera.hpp"
#include "world.hpp"

using namespace Game::Config;

namespace Game {
  World::World(const Camera &camera) : camera(camera), skybox(blockManager.getMeshData(Blocks::MeshId::Default)) {
    Logger::logInfo(Logger::Context::Game, "Generating terrain with a seed of %d.", seed);
    shader.use();
    shader.updateTexture(Renderer::loadPng("./res/images/blocks.png"));

    for (size_t threadCount = 0; threadCount < 1; threadCount++) {
      std::thread terrainLoader(&World::loadTerrain, this);
      builderThreads.push_back(std::move(terrainLoader));
    }
  }

  World::~World() {
    chunkBuilder.notify_all();
    running = false;
    generating = false;

    for (auto &thread: builderThreads) {
      if (thread.joinable())
        thread.join();
    }
  }

  void World::render() {
    skybox.render();

    shader.use();
    shader.updateProjectionMatrix(camera.getProjectionMatrix());
    shader.updateViewMatrix(camera.getViewMatrix());

    if (camera.moving()) {
      generating = true;
      chunkBuilder.notify_all();
    } else {
      generating = false;
    }

    std::vector<ChunkPosition> localDeleteQueue;
    std::unique_lock queueLock(deleteQueueMutex);
    deleteQueue.swap(localDeleteQueue);
    queueLock.unlock();

    std::unique_lock deleteLock(chunkBuilderMutex);
    for (auto &position: localDeleteQueue) {
      if (chunkMap.contains(position)) {
        auto &chunk = chunkMap.at(position);
        chunk.deleteBuffers();
        chunkMap.erase(position);
      }
    }
    deleteLock.unlock();

    // Not really the most efficient way of sorting but works for now
    std::unique_lock renderLock(chunkBuilderMutex);
    std::multimap<float, ChunkPosition> waterMeshes;
    for (auto &[position, chunk]: chunkMap) {
      static constexpr int halfChunkSize = ChunkSize / 2;
      const glm::vec3 waterMeshPosition = glm::vec3(
        position.x + halfChunkSize,
        WaterLevel,
        position.y + halfChunkSize
      );
      const float distance = glm::length(camera.getPosition() - waterMeshPosition);
      waterMeshes.insert({distance, position});
    }

    for (std::map<float, ChunkPosition>::reverse_iterator it = waterMeshes.rbegin(); it != waterMeshes.rend(); ++it) {
      const ChunkPosition &chunkPosition = it->second;
      if (chunkMap.contains(chunkPosition)) {
        auto &chunk = chunkMap.at(chunkPosition);
        shader.updateModelMatrix(chunk.getModelMatrix());
        chunk.renderBlocks();
        chunk.renderWater();
      }
    }
  }

  void World::loadTerrain() {
    while (running) {
      std::unique_lock chunkBuilderLock(chunkBuilderMutex);
      chunkBuilder.wait(chunkBuilderLock, [this] { return generating || !running; });

      static constexpr int halfRenderDistance = (RenderDistance / 2) * ChunkSize;
      const glm::ivec3 &cameraPosition = camera.getPosition();
      const glm::ivec2 cameraGridPosition = glm::ivec2(
        (cameraPosition.x / ChunkSize) * ChunkSize,
        (cameraPosition.z / ChunkSize) * ChunkSize
      );

      const int minimumX = cameraGridPosition.x - halfRenderDistance;
      const int maximumX = cameraGridPosition.x + halfRenderDistance;
      const int minimumZ = cameraGridPosition.y - halfRenderDistance;
      const int maximumZ = cameraGridPosition.y + halfRenderDistance;

      for (const auto &[position, chunk] : chunkMap) {
        const bool cullChunk = position.x < minimumX || position.x > maximumX || position.y < minimumZ || position.y > maximumZ;
        if (cullChunk) {
          std::lock_guard lock(deleteQueueMutex);
          deleteQueue.push_back(position);
        }
      }
      chunkBuilderLock.unlock();

      for (size_t chunkIndex = 0; chunkIndex < RenderDistance * RenderDistance; chunkIndex++) {
        const int x = cameraGridPosition.x + (chunkIndex % RenderDistance) * ChunkSize - halfRenderDistance;
        const int z = cameraGridPosition.y + ((chunkIndex / RenderDistance) % RenderDistance) * ChunkSize -
                      halfRenderDistance;
        const ChunkPosition position(x, z);

        if (!chunkMap.contains(position)) {
          std::this_thread::sleep_for(std::chrono::milliseconds(1));
          std::lock_guard lock(chunkBuilderMutex);
          chunkMap.emplace(position, Chunk(position, generateHeightMap(position / static_cast<int>(ChunkSize)), *this));
        }
      }
    }
  }

  Blocks::BlockId World::getBlockId(const glm::ivec3 &position) {
    return evaluateBlockType(position.y, terrainNoise(glm::ivec2(position.x, position.z)));
  }

  Blocks::BlockId World::evaluateBlockType(int y, int noiseHeight) {
    const bool inChunkHeight = y >= MinChunkHeight && y < MaxChunkHeight;
    if (!inChunkHeight)
      return Blocks::BlockId::Air;

    if (y == MinChunkHeight)
      return Blocks::BlockId::Bedrock;

    if (y == MinChunkHeight + 1)
      return Blocks::BlockId::Dirt;

    if (y == noiseHeight)
      return Blocks::BlockId::Grass;

    if (y < noiseHeight) {
      if (y <= noiseHeight - 10)
        return Blocks::BlockId::Stone;

      return Blocks::BlockId::Dirt;
    }

    if (y == WaterLevel)
      return Blocks::BlockId::Water;

    return Blocks::BlockId::Air;
  }

  std::vector<int> World::generateHeightMap(const glm::ivec2 &position) {
    std::vector<int> heightMap;
    for (size_t blockCount = 0; blockCount < ChunkSize * ChunkSize; blockCount++) {
      const int x = blockCount % ChunkSize + (position.x * ChunkSize);
      const int z = (blockCount / ChunkSize) % ChunkSize + (position.y * ChunkSize);
      const int y = terrainNoise(glm::ivec2(x, z));
      heightMap.push_back(y);
    }
    return heightMap;
  }

  int World::generateSeed() {
    srand(time(nullptr));
    static constexpr long seedRange = static_cast<long>(10e8);
    return rand() % (2 * seedRange + 1) - seedRange;
  }

  int World::terrainNoise(const glm::ivec2 &position) const {
    int noise = NoiseHeightOffset;
    float scale = NoiseBaseScale;
    float amplitude = NoiseBaseAmplitude;
    for (size_t octave = 1; octave < NoiseOctaves + 1; octave++) {
      scale *= 2.0f;
      amplitude *= 0.5f;
      noise += noise2d(position, glm::vec2(scale, scale), amplitude);
    }

    return noise;
  }

  int World::noise2d(const glm::ivec2 &position, const glm::vec2 &scale, float amplitude) const {
    const auto x = static_cast<double>(position.x);
    const auto y = static_cast<double>(position.y);
    return static_cast<int>(ceilf(noiseGenerator.eval(x * scale.x, y * scale.y) * amplitude));
  }
}
