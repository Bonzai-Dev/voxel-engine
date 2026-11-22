#pragma once
#include <core/shared_pointer.hpp>

namespace Engine::Renderer {
  class RendererResource: public RefCounted {
    public:
      RendererResource() = default;

      ~RendererResource() override = default;
  };
}