#pragma once
#include <core/rhi/rhi.hpp>

namespace Core::Renderer {
  class Renderer {
    public:
      Renderer();

      virtual ~Renderer();

    private:

  };

  class Renderer3D final: public Renderer {
    public:
      Renderer3D();

      ~Renderer3D() override;

    private:
      RHI::Device *renderingDevice = nullptr;
  };
}