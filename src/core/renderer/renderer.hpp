#pragma once
#include <vector>
#include <NRI.h>

#include "Extensions/NRIHelper.h"
#include "Extensions/NRILowLatency.h"
#include "Extensions/NRIMeshShader.h"
#include "Extensions/NRIRayTracing.h"
#include "Extensions/NRIStreamer.h"
#include "Extensions/NRISwapChain.h"
#include "Extensions/NRIUpscaler.h"

namespace Core::Renderer {
  struct QueuedFrame {
    nri::CommandAllocator* commandAllocator;
    nri::CommandBuffer* commandBuffer;
    nri::Descriptor* constantBufferView;
    nri::DescriptorSet* constantBufferDescriptorSet;
    uint64_t constantBufferViewOffset;
  };

  struct SwapChainTexture {
    nri::Fence* acquireSemaphore;
    nri::Fence* releaseSemaphore;
    nri::Texture* texture;
    nri::Descriptor* colorAttachment;
    nri::Format attachmentFormat;
  };

  struct NRIInterface
    : public nri::CoreInterface,
      public nri::HelperInterface,
      public nri::LowLatencyInterface,
      public nri::MeshShaderInterface,
      public nri::RayTracingInterface,
      public nri::StreamerInterface,
      public nri::SwapChainInterface,
      public nri::UpscalerInterface {
    inline bool HasCore() const {
      return GetDeviceDesc != nullptr;
    }

    inline bool HasHelper() const {
      return CalculateAllocationNumber != nullptr;
    }

    inline bool HasLowLatency() const {
      return SetLatencySleepMode != nullptr;
    }

    inline bool HasMeshShader() const {
      return CmdDrawMeshTasks != nullptr;
    }

    inline bool HasRayTracing() const {
      return CreateRayTracingPipeline != nullptr;
    }

    inline bool HasStreamer() const {
      return CreateStreamer != nullptr;
    }

    inline bool HasSwapChain() const {
      return CreateSwapChain != nullptr;
    }

    inline bool HasUpscaler() const {
      return CreateUpscaler != nullptr;
    }
  };

  class Renderer {
    public:
      Renderer();

      virtual ~Renderer();

    private:
      NRIInterface NRI = {};
      nri::Device* m_Device = nullptr;
      nri::Streamer* m_Streamer = nullptr;
      nri::SwapChain* m_SwapChain = nullptr;
      nri::Queue* m_GraphicsQueue = nullptr;
      nri::Fence* m_FrameFence = nullptr;
      nri::DescriptorPool* m_DescriptorPool = nullptr;
      nri::PipelineLayout* m_PipelineLayout = nullptr;
      nri::Pipeline* m_Pipeline = nullptr;
      nri::Pipeline* m_PipelineMultiview = nullptr;
      nri::PipelineCache* m_PipelineCache = nullptr;
      nri::DescriptorSet* m_TextureDescriptorSet = nullptr;
      nri::Descriptor* m_TextureShaderResource = nullptr;
      nri::Buffer* m_ConstantBuffer = nullptr;
      nri::Buffer* m_GeometryBuffer = nullptr;
      nri::Texture* m_Texture = nullptr;

      std::vector<QueuedFrame> m_QueuedFrames = {};
      std::vector<SwapChainTexture> m_SwapChainTextures;
      std::vector<nri::Memory*> m_MemoryAllocations;

      uint64_t m_GeometryOffset = 0;
      bool m_Multiview = false;
      float m_Transparency = 1.0f;
      float m_Scale = 1.0f;
  };
}
