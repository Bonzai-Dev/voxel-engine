#pragma once
#include <glm/vec3.hpp>

namespace Game {
  // Cannot build above MaxChunkHeight and below MinChunkHeight
  inline constexpr int MaxChunkHeight = 320;
  inline constexpr int MinChunkHeight = -64;
  inline constexpr int ChunkSize = 16;
  inline constexpr int ChunkHeight = MaxChunkHeight - MinChunkHeight;
  inline constexpr size_t TotalChunkBlocks = ChunkSize * ChunkSize * ChunkHeight;

  inline constexpr size_t RenderDistance = 2;

  inline constexpr auto Up = glm::ivec3(0, 1, 0);
  inline constexpr auto Down = glm::ivec3(0, -1, 0);
  inline constexpr auto Forward = glm::ivec3(0, 0, 1);
  inline constexpr auto Backward = glm::ivec3(0, 0, -1);
  inline constexpr auto Left = glm::ivec3(-1, 0, 0);
  inline constexpr auto Right = glm::ivec3(1, 0, 0);

  class World {
    public:
      World();

      // Block GetBlock(int x, int y, int z);

    private:
  };
}
