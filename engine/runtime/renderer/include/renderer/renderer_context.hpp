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
  public nri::CoreInterface,
  public nri::DescriptorHeapInterface,
  public nri::HelperInterface,
  public nri::LowLatencyInterface,
  public nri::MeshShaderInterface,
  public nri::RayTracingInterface,
  public nri::StreamerInterface,
  public nri::SwapChainInterface,
  public nri::UpscalerInterface {
    public:
      explicit RendererContext();

      ~RendererContext() override;

      nri::Device &getDevice() const { return *renderingDevice; }

    private:
      nri::Device *renderingDevice = nullptr;
  };
}
