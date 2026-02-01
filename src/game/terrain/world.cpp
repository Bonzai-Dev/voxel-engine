#include <thread>
#include <map>
#include <core/logger.hpp>
#include "../camera.hpp"
#include "world.hpp"

#include <iostream>

namespace Game {
  World::World(const Camera &camera) : camera(camera), skybox(blockManager.getMeshData(Blocks::MeshId::Skybox)) {
    Logger::logInfo(Logger::Context::Game, "Generating terrain with a seed of %d.", seed);
    shader.use();
    shader.updateTexture(Renderer::loadTexture("./res/images/blocks.png"));
    shader.setAmbientColor(Config::getAmbientLightColor());
    shader.setSunDirection(Config::getSunDirection());
    shader.setSunColor(Config::getSunColor());
    shader.setSunBrightness(Config::getSunBrightness());

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
    static const int seaLevel = Config::getSeaLevel();
    static const int chunkSize = Config::getChunkSize();
    static const int halfChunkSize = chunkSize / 2;

    shader.use();
    shader.setProjectionMatrix(camera.getProjectionMatrix());
    shader.setViewMatrix(camera.getViewMatrix());

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
      const glm::vec3 waterMeshPosition = glm::vec3(
        position.x + halfChunkSize,
        seaLevel,
        position.y + halfChunkSize
      );
      const float distance = glm::length(camera.getPosition() - waterMeshPosition);
      waterMeshes.insert({distance, position});
    }

    for (std::map<float, ChunkPosition>::reverse_iterator it = waterMeshes.rbegin(); it != waterMeshes.rend(); ++it) {
      const ChunkPosition &chunkPosition = it->second;
      if (chunkMap.contains(chunkPosition)) {
        auto &chunk = chunkMap.at(chunkPosition);
        shader.setModelMatrix(chunk.getModelMatrix());
        chunk.renderBlocks();
        chunk.renderWater();
      }
    }
    skybox.render(camera);
  }

  void World::loadTerrain() {
    static const int renderDistance = Config::getRenderDistance();
    static const int chunkSize  = Config::getChunkSize();
    static const int halfRenderDistance = (renderDistance / 2) * chunkSize;

    while (running) {
      std::unique_lock chunkBuilderLock(chunkBuilderMutex);
      chunkBuilder.wait(chunkBuilderLock, [this] { return generating || !running; });

      const glm::ivec3 &cameraPosition = camera.getPosition();
      const glm::ivec2 cameraGridPosition = glm::ivec2(
        (cameraPosition.x / chunkSize) * chunkSize,
        (cameraPosition.z / chunkSize) * chunkSize
      );

      const int minimumX = cameraGridPosition.x - halfRenderDistance;
      const int maximumX = cameraGridPosition.x + halfRenderDistance;
      const int minimumZ = cameraGridPosition.y - halfRenderDistance;
      const int maximumZ = cameraGridPosition.y + halfRenderDistance;

      for (const auto &[position, chunk]: chunkMap) {
        const bool cullChunk = position.x < minimumX || position.x > maximumX || position.y < minimumZ || position.y >
                               maximumZ;
        if (cullChunk) {
          std::lock_guard lock(deleteQueueMutex);
          deleteQueue.push_back(position);
        }
      }
      chunkBuilderLock.unlock();

      for (size_t chunkIndex = 0; chunkIndex < renderDistance * renderDistance; chunkIndex++) {
        const int x = cameraGridPosition.x + (chunkIndex % renderDistance) * chunkSize - halfRenderDistance;
        const int z = cameraGridPosition.y + ((chunkIndex / renderDistance) % renderDistance) * chunkSize -
                      halfRenderDistance;
        const ChunkPosition position(x, z);

        if (!chunkMap.contains(position)) {
          std::this_thread::sleep_for(std::chrono::milliseconds(1));
          std::lock_guard lock(chunkBuilderMutex);
          chunkMap.emplace(position, Chunk(position, generateHeightMap(position / chunkSize), *this));
        }
      }
    }
  }

  Blocks::BlockId World::getBlockId(const glm::ivec3 &position) {
    return evaluateBlockType(position.y, terrainNoise(glm::ivec2(position.x, position.z)));
  }

  Blocks::BlockId World::evaluateBlockType(int y, int noiseHeight) {
    static const int minChunkHeight = Config::getMinChunkHeight();
    static const int maxChunkHeight = Config::getMaxChunkHeight();
    static const int seaLevel = Config::getSeaLevel();
    static const int sandLevel = Config::getSandLevel();
    static const int snowHeight = Config::getSnowHeight();

    using namespace Blocks;
    const bool inChunkHeight = y >= minChunkHeight && y < maxChunkHeight;
    if (!inChunkHeight)
      return BlockId::Air;

    BlockId blockId = BlockId::Air;
    if (y == minChunkHeight)
      return BlockId::Bedrock;

    if (y == minChunkHeight + 1 || y < noiseHeight)
      blockId = BlockId::Dirt;

    if (y == noiseHeight)
      blockId = BlockId::Grass;

    if (y < seaLevel && blockId == BlockId::Air)
      blockId = BlockId::Water;
    else if (y > sandLevel - randomInt(5, 6) && y < sandLevel + randomInt(0, 1) && (blockId == BlockId::Dirt || blockId == BlockId::Grass))
      blockId = BlockId::Sand;
    else if (y > minChunkHeight && y < seaLevel - randomInt(0, 2) && blockId == BlockId::Dirt)
      blockId = BlockId::Gravel;
    else if (y > minChunkHeight && y < seaLevel)
      blockId = BlockId::Dirt;

    if (y > snowHeight - randomInt(0, 12) && y <= noiseHeight && (blockId == BlockId::Grass || blockId == BlockId::Dirt))
      blockId = BlockId::Snow;
    else if (y > snowHeight - 20 - randomInt(0, 8) && y <= noiseHeight && (blockId == BlockId::Grass || blockId == BlockId::Dirt))
      blockId = BlockId::Stone;

    return blockId;
  }

  std::vector<int> World::generateHeightMap(const glm::ivec2 &position) {
    static const size_t chunkSize = Config::getChunkSize();
    std::vector<int> heightMap;
    for (size_t blockCount = 0; blockCount < chunkSize * chunkSize; blockCount++) {
      const int x = blockCount % chunkSize + (position.x * chunkSize);
      const int z = (blockCount / chunkSize) % chunkSize + (position.y * chunkSize);
      const int y = terrainNoise(glm::ivec2(x, z));
      heightMap.push_back(y);
    }
    return heightMap;
  }

  int World::generateSeed() {
    static constexpr long seedRange = static_cast<long>(10e8);
    return randomInt(-seedRange, seedRange);
  }

  int World::terrainNoise(const glm::ivec2 &position) const {
    static const int noiseHeightOffset = Config::getNoiseHeightOffset();
    static const float noiseBaseScale = Config::getNoiseBaseScale();
    static const float noiseBaseAmplitude = Config::getNoiseBaseAmplitude();
    static const int noiseOctaves = Config::getNoiseOctaves();

    int noise = noiseHeightOffset;
    float scale = noiseBaseScale;
    float amplitude = noiseBaseAmplitude;
    for (size_t octave = 1; octave < noiseOctaves + 1; octave++) {
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

  int World::randomInt(int minimum, int maximum) {
    static bool seeded = false;
    if (!seeded && Config::randomizeSeed()) {
      srand(time(nullptr));
      seeded = true;
    }

    return rand() % (maximum - minimum + 1) + minimum;
  }
}
