#pragma once
#include <NRI.h>
#include "resource.hpp"
#include "Extensions/NRIDescriptorHeap.h"
#include "Extensions/NRIHelper.h"
#include "Extensions/NRILowLatency.h"
#include "Extensions/NRIMeshShader.h"
#include "Extensions/NRIRayTracing.h"
#include "Extensions/NRIStreamer.h"
#include "Extensions/NRISwapChain.h"
#include "Extensions/NRIUpscaler.h"
#include <core/logging.hpp>

namespace Engine::Application {
  class GraphicsWindow;
}

namespace Engine::Application {
  class PlatformWindow;
}

namespace Engine::Renderer {
  ENGINE_DEFINE_LOG_CATEGORY(rendererLogger, "renderer");

  class RendererContext: 
  public RendererResource,
  public RHI::CoreInterface,
  public RHI::DescriptorHeapInterface,
  public RHI::HelperInterface,
  public RHI::LowLatencyInterface,
  public RHI::MeshShaderInterface,
  public RHI::RayTracingInterface,
  public RHI::StreamerInterface,
  public RHI::SwapChainInterface,
  public RHI::UpscalerInterface {
    public:
      explicit RendererContext();

      ~RendererContext() override;

      RHI::Device &getDevice() const { return *renderingDevice; }

    private:
      RHI::Device *renderingDevice = nullptr;
  };
}
