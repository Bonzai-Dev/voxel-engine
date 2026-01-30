#pragma once
#include <condition_variable>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <glm/vec3.hpp>
#include <core/open_simplex2s.hpp>
#include <glm/vec2.hpp>
#include <app/camera.hpp>
#include <app/shaders/default.hpp>
#include "chunk.hpp"
#include "blocks/block_manager.hpp"

namespace Game {
  class ChunkPosition : public glm::ivec2 {
    friend bool operator==(const ChunkPosition& a, const ChunkPosition& b) {
      return hash(a) == hash(b);
    }

    public:
      static unsigned long long hash(const glm::ivec2 &position) {
        unsigned long long hash = 0;
        hash = hash * 257 + position.x;
        hash %= 1000000007;

        hash = hash * 257 + position.y;
        hash %= 1000000007;
        return hash;
      }

    ChunkPosition(int x, int y) {
      this->x = x;
      this->y = y;
    }

    ~ChunkPosition() = default;
  };
}

template <>
struct std::hash<Game::ChunkPosition> {
  size_t operator()(const Game::ChunkPosition& position) const noexcept {
    return Game::ChunkPosition::hash(position);
  }
};

namespace Game {
  class BlockManager;

  inline constexpr auto Up = glm::ivec3(0, 1, 0);
  inline constexpr auto Down = glm::ivec3(0, -1, 0);
  inline constexpr auto Forward = glm::ivec3(0, 0, 1);
  inline constexpr auto Backward = glm::ivec3(0, 0, -1);
  inline constexpr auto Left = glm::ivec3(-1, 0, 0);
  inline constexpr auto Right = glm::ivec3(1, 0, 0);

  class World {
    public:
      World(const Camera &camera);

      ~World();

      const int &getSeed() const { return seed; }

      void render();

      const Blocks::BlockManager &getBlockManager() const { return blockManager; }

      Blocks::BlockId getBlockId(const glm::ivec3 &position);

      Blocks::BlockId evaluateBlockType(int y, int noiseHeight);

    private:
      Blocks::BlockManager blockManager;
      Core::OpenSimplexNoise::Noise noiseGenerator = Core::OpenSimplexNoise::Noise(getSeed());
      const Camera &camera;
      Shader::Default shader;

      std::vector<std::thread> builderThreads;
      std::unordered_map<ChunkPosition, Chunk> chunkMap;
      std::vector<ChunkPosition> deleteQueue;
      std::mutex chunkBuilderMutex;
      std::mutex deleteQueueMutex;
      std::condition_variable chunkBuilder;

      bool generating = true;
      bool running = true;

      void loadTerrain();

      int noise2d(const glm::ivec2 &position, const glm::vec2 &scale, float amplitude) const;

      int terrainNoise(const glm::ivec2 &position) const;

      std::vector<int> generateHeightMap(const glm::ivec2 &position);

      static int generateSeed();

      int seed = generateSeed();
  };
}
