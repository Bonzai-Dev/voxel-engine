#include <NRI.h>
#include <core/logger.hpp>
#include <cinttypes>
#include "Extensions/NRIDeviceCreation.h"
#include "renderer.hpp"

#include <array>
#include <fstream>

constexpr const char *PSO_CACHE_PATH = "pso_cache.bin";

constexpr uint32_t VIEW_MASK = 0b11;
constexpr nri::Color32f COLOR_0 = {1.0f, 1.0f, 0.0f, 1.0f};
constexpr nri::Color32f COLOR_1 = {0.46f, 0.72f, 0.0f, 1.0f};

struct ConstantBufferLayout {
  float color[3];
  float scale;
};

struct Vertex {
  float position[2];
  float uv[2];
};

static const Vertex g_VertexData[] = {
  {{-0.71f, -0.50f}, {0.0f, 0.0f}},
  {{0.00f, 0.71f}, {1.0f, 1.0f}},
  {{0.71f, -0.50f}, {0.0f, 1.0f}},
};

static const uint16_t g_IndexData[] = {0, 1, 2};

inline uint8_t GetQueuedFrameNum() {
  // return m_Vsync ? 2 : 3;
  return 2;
}

inline uint8_t GetOptimalSwapChainTextureNum() {
  return GetQueuedFrameNum() + 1;
}

namespace Core::Renderer {
  Renderer::Renderer() {
    // Adapters
    nri::AdapterDesc adapterDesc[2] = {};
    uint32_t adapterDescsNum = 0;
    nri::nriEnumerateAdapters(adapterDesc, adapterDescsNum);

    // Device
    nri::DeviceCreationDesc deviceCreationDesc = {};
    // deviceCreationDesc.graphicsAPI = graphicsAPI;
    // deviceCreationDesc.enableGraphicsAPIValidation = m_DebugAPI;
    // deviceCreationDesc.enableNRIValidation = m_DebugNRI;
    // deviceCreationDesc.enableD3D11CommandBufferEmulation = D3D11_ENABLE_COMMAND_BUFFER_EMULATION;
    // deviceCreationDesc.disableD3D12EnhancedBarriers = D3D12_DISABLE_ENHANCED_BARRIERS;
    // deviceCreationDesc.vkBindingOffsets = VK_BINDING_OFFSETS;
    // deviceCreationDesc.adapterDesc = &adapterDesc[std::min(m_AdapterIndex, adapterDescsNum - 1)];
    // deviceCreationDesc.allocationCallbacks = m_AllocationCallbacks;

    // if (!(deviceCreationDesc.adapterDesc->supportedGraphicsAPIs & graphicsAPI))
    //   exit(0);

    nri::nriCreateDevice(deviceCreationDesc, m_Device);

    // NRI
    nri::nriGetInterface(*m_Device, NRI_INTERFACE(nri::CoreInterface), (nri::CoreInterface *) &NRI);
    nri::nriGetInterface(*m_Device, NRI_INTERFACE(nri::HelperInterface),
                                              (nri::HelperInterface *) &NRI);
    nri::nriGetInterface(*m_Device, NRI_INTERFACE(nri::StreamerInterface),
                                              (nri::StreamerInterface *) &NRI);
    nri::nriGetInterface(*m_Device, NRI_INTERFACE(nri::SwapChainInterface),
                                              (nri::SwapChainInterface *) &NRI);

    // Create streamer
    nri::StreamerDesc streamerDesc = {};
    streamerDesc.dynamicBufferMemoryLocation = nri::MemoryLocation::HOST_UPLOAD;
    streamerDesc.dynamicBufferDesc = {0, 0, nri::BufferUsageBits::VERTEX_BUFFER | nri::BufferUsageBits::INDEX_BUFFER};
    streamerDesc.constantBufferMemoryLocation = nri::MemoryLocation::HOST_UPLOAD;
    streamerDesc.queuedFrameNum = GetQueuedFrameNum();
    // streamerDesc.hostDataCapacity = IMGUI_HOST_DATA_CAPACITY;
    NRI.CreateStreamer(*m_Device, streamerDesc, m_Streamer);

    // Command queue
    NRI.GetQueue(*m_Device, nri::QueueType::GRAPHICS, 0, m_GraphicsQueue);

    // Fences
    NRI.CreateFence(*m_Device, 0, m_FrameFence);

    const nri::DeviceDesc &deviceDesc = NRI.GetDeviceDesc(*m_Device);
    {
      // Pipeline cache
      nri::PipelineCacheDesc cacheDesc = {};
      std::vector<uint8_t> blob;

      if (deviceDesc.features.pipelineCache) {
        std::ifstream f(PSO_CACHE_PATH, std::ios::binary | std::ios::ate);
        if (f) {
          uint64_t size = (uint64_t) f.tellg();
          blob.resize(size);

          f.seekg(0).read((char *) blob.data(), (std::streamsize) size);

          cacheDesc.data = blob.data();
          cacheDesc.size = size;

          printf("Pipeline cache: loaded %" PRIu64 " bytes from '%s'\n", size, PSO_CACHE_PATH);
        } else
          printf("Pipeline cache: '%s' not found, starting empty\n", PSO_CACHE_PATH);
      } else
        printf("Pipeline cache: unsupported\n");

      nri::Result result = NRI.CreatePipelineCache(*m_Device, cacheDesc, m_PipelineCache);
      if (result == nri::Result::OUT_OF_DATE) {
        printf("Pipeline cache: supplied blob is stale, recreating empty\n");

        nri::PipelineCacheDesc empty = {};
        NRI.CreatePipelineCache(*m_Device, empty, m_PipelineCache);
      }
    }


  }

  Renderer::~Renderer() {
    if (NRI.HasCore()) {
      NRI.DeviceWaitIdle(m_Device);

      for (QueuedFrame &queuedFrame: m_QueuedFrames) {
        NRI.DestroyCommandBuffer(queuedFrame.commandBuffer);
        NRI.DestroyCommandAllocator(queuedFrame.commandAllocator);
        NRI.DestroyDescriptor(queuedFrame.constantBufferView);
      }

      for (SwapChainTexture &swapChainTexture: m_SwapChainTextures) {
        NRI.DestroyFence(swapChainTexture.acquireSemaphore);
        NRI.DestroyFence(swapChainTexture.releaseSemaphore);
        NRI.DestroyDescriptor(swapChainTexture.colorAttachment);
      }

      NRI.DestroyPipeline(m_Pipeline);
      NRI.DestroyPipeline(m_PipelineMultiview);
      NRI.DestroyPipelineLayout(m_PipelineLayout);
      NRI.DestroyDescriptor(m_TextureShaderResource);
      NRI.DestroyBuffer(m_ConstantBuffer);
      NRI.DestroyBuffer(m_GeometryBuffer);
      NRI.DestroyTexture(m_Texture);
      NRI.DestroyDescriptorPool(m_DescriptorPool);
      NRI.DestroyFence(m_FrameFence);

      for (nri::Memory *memory: m_MemoryAllocations)
        NRI.FreeMemory(memory);

      // Pipeline cache
      if (m_PipelineCache) {
        uint64_t size = 0;
        // (NRI.GetPipelineCacheData(*m_PipelineCache, nullptr, size));
        NRI.GetPipelineCacheData(*m_PipelineCache, nullptr, size);

        if (size > 0) {
          std::vector<uint8_t> blob(size);
          // (NRI.GetPipelineCacheData(*m_PipelineCache, blob.data(), size));

          std::ofstream(PSO_CACHE_PATH, std::ios::binary).write((const char *) blob.data(), (std::streamsize) size);
          printf("Pipeline cache: saved %" PRIu64 " bytes to '%s'\n", size, PSO_CACHE_PATH);
        }

        NRI.DestroyPipelineCache(m_PipelineCache);
      }
    }

    if (NRI.HasSwapChain())
      NRI.DestroySwapChain(m_SwapChain);

    if (NRI.HasStreamer())
      NRI.DestroyStreamer(m_Streamer);

    // DestroyImgui();

    nri::nriDestroyDevice(m_Device);
  }
}
