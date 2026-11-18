#pragma once
#include <engine/memory/intrusive_ptr.hpp>

namespace Engine::Renderer {
  class RendererResource: public RefCounted {
    public:
      RendererResource() = default;

      ~RendererResource() override = default;
  };
}