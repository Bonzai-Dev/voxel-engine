#pragma once

namespace Game::Config {
  constexpr inline float CameraSensitivity = 0.25f;
  constexpr inline float CameraSpeed = 50.0f;

  // Cannot build above MaxChunkHeight and below MinChunkHeight
  constexpr inline int MaxChunkHeight = 50;
  constexpr inline int MinChunkHeight = -10;
  constexpr inline size_t ChunkSize = 16;
  constexpr inline size_t ChunkHeight = MaxChunkHeight - MinChunkHeight;
  constexpr inline size_t TotalChunkBlocks = ChunkSize * ChunkSize * ChunkHeight;
  constexpr inline int WaterLevel = -2;

  // How many chunks to render in front of the camera
  constexpr inline int RenderDistance = 2;

  constexpr inline int NoiseHeightOffset = 5;
  constexpr inline int NoiseOctaves = 6;
  constexpr inline float NoiseBaseScale = 0.007f;
  constexpr inline float NoiseBaseAmplitude = 50.0f;
}
