#pragma once
#include <math/math.hpp>

namespace Engine::Renderer {
  struct Vertex {
    float3 position;
    float2 uv;
    float3 normal;
    float4 tangent;
    float4 color;
  };


}