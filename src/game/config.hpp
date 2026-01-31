#pragma once

namespace Game::Config {
  constexpr inline glm::vec3 AmbientLightColor = glm::vec3(100, 82, 63);
  constexpr inline float CameraSensitivity = 0.25f;
  constexpr inline float CameraSpeed = 50.0f;

  // Whether to randomize the world seed on each launch
  constexpr inline bool randomize = true;

  // Cannot build above MaxChunkHeight and below MinChunkHeight
  constexpr inline int MaxChunkHeight = 50;
  constexpr inline int MinChunkHeight = -10;
  constexpr inline size_t ChunkSize = 16;
  constexpr inline size_t ChunkHeight = MaxChunkHeight - MinChunkHeight;
  constexpr inline size_t TotalChunkBlocks = ChunkSize * ChunkSize * ChunkHeight;
  constexpr inline int SeaLevel = -2;
  constexpr inline int SandLevel = SeaLevel + 2;

  // How many chunks to render in front of the camera
  constexpr inline int RenderDistance = 30;

  constexpr inline int NoiseHeightOffset = 5;
  constexpr inline int NoiseOctaves = 6;
  constexpr inline float NoiseBaseScale = 0.007f;
  constexpr inline float NoiseBaseAmplitude = 50.0f;
}
