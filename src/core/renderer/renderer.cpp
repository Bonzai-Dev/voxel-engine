#include <core/logger.hpp>
#include <core/rhi/rhi.hpp>
#include "core/rhi/stl/allocator.hpp"
#include "renderer.hpp"

namespace Core::Renderer {
  Renderer::Renderer() {
  }

  Renderer::~Renderer() {
  }

  Renderer3D::Renderer3D() {
    RHI::createDevice({}, renderingDevice);
  }

  Renderer3D::~Renderer3D() {
    RHI::destroy(renderingDevice->allocationCallbacks, renderingDevice);
  }
}
