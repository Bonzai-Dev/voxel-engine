#pragma once
#include <cstdint>

namespace Engine::Application {
  struct DisplayInfo {
    uint32_t displayId;
    uint32_t width;
    uint32_t height;
  };
}