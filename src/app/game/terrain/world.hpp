#pragma once
#include <thread>
#include <mutex>
#include <glm/vec3.hpp>
#include <core/open_simplex2s.hpp>
#include <glm/vec2.hpp>
#include <core/renderer/camera/camera.hpp>
#include <app/game/shaders/default.hpp>
#include "chunk.hpp"
#include "blocks/block_manager.hpp"

namespace Game {
  class ChunkPosition : public glm::ivec2 {
    friend bool operator==(const ChunkPosition& a, const ChunkPosition& b) {
      return hash(a) == hash(b);
    }

    public:
      static int hash(const glm::ivec2 &position) {
        return (position.x ^ 0x1f) + (position.y << ChunkSize);
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

  // How many chunks to render in front of the camera
  inline constexpr int RenderDistance = 30;

  inline constexpr auto Up = glm::ivec3(0, 1, 0);
  inline constexpr auto Down = glm::ivec3(0, -1, 0);
  inline constexpr auto Forward = glm::ivec3(0, 0, 1);
  inline constexpr auto Backward = glm::ivec3(0, 0, -1);
  inline constexpr auto Left = glm::ivec3(-1, 0, 0);
  inline constexpr auto Right = glm::ivec3(1, 0, 0);

  class World {
    public:
      World(const Renderer::Camera &camera);

      ~World();

      const int &getSeed() const { return seed; }

      void render();

      const Blocks::BlockManager &getBlockManager() const { return blockManager; }

    private:
      Blocks::BlockManager blockManager;
      Core::OpenSimplexNoise::Noise noiseGenerator = Core::OpenSimplexNoise::Noise(getSeed());
      const Renderer::Camera &camera;
      Shader::Default shader;
      std::unordered_map<ChunkPosition, Chunk> chunks;
      std::vector<std::thread> chunkBuilder;
      std::mutex chunkBuilderMutex;

      bool running = true;

      void loadTerrain();

      int noise2d(const glm::ivec2 &position, const glm::vec2 &scale, float amplitude) const;

      int terrainNoise(const glm::ivec2 &position) const;

      std::vector<int> generateHeightMap(const glm::ivec2 &position);

      static int generateSeed();

      int seed = generateSeed();
  };
}
