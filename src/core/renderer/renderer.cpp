#include <core/logger.hpp>
#include <core/rhi/rhi.hpp>
#include "renderer.hpp"

namespace {
  using namespace Core;

  constexpr const char *PSO_CACHE_PATH = "pso_cache.bin";

  constexpr uint32_t VIEW_MASK = 0b11;
  constexpr RHI::Color32f COLOR_0 = {1.0f, 1.0f, 0.0f, 1.0f};
  constexpr RHI::Color32f COLOR_1 = {0.46f, 0.72f, 0.0f, 1.0f};

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

  inline uint8_t getQueuedFrameCount() {
    // return m_Vsync ? 2 : 3;
    return 3;
  }

  inline uint8_t getOptimalSwapChainTextureNum() {
    return getQueuedFrameCount() + 1;
  }
}

namespace Core::Renderer {
  Renderer::Renderer() {
    RHI::DeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.enableGraphicsAPIValidation = true;
    RHI::createDevice(deviceCreateInfo, renderingDevice);

    RHI::getInterface(*renderingDevice, coreInterface);
    RHI::getInterface(*renderingDevice, swapChainInterface);

    {
      // Pipeline layout
      RHI::SamplerInfo samplerDesc = {};
      samplerDesc.addressModes = {RHI::AddressMode::MirroredRepeat, RHI::AddressMode::MirroredRepeat};
      samplerDesc.filters = {RHI::Filter::Linear, RHI::Filter::Linear, RHI::Filter::Linear};
      samplerDesc.anisotropy = 4;
      samplerDesc.mipMax = 16.0f;

      RHI::RootConstantInfo rootConstant = {1, sizeof(float), RHI::StageBits::FragmentShader};
      RHI::RootSamplerInfo rootSampler = {0, samplerDesc, RHI::StageBits::FragmentShader};
      RHI::DescriptorRangeDesc setConstantBuffer = {0, 1, RHI::DescriptorType::ConstantBuffer, RHI::StageBits::All};
      RHI::DescriptorRangeDesc setTexture = {0, 1, RHI::DescriptorType::Texture, RHI::StageBits::FragmentShader};

      std::array descriptorSetDescs = {
        RHI::DescriptorSetInfo{0, &setConstantBuffer, 1},
        RHI::DescriptorSetInfo{1, &setTexture, 1},
      };

      RHI::PipelineLayoutInfo pipelineLayoutDesc = {};
      pipelineLayoutDesc.rootRegisterSpace = 2; // see shader
      pipelineLayoutDesc.rootConstantCount = 1;
      pipelineLayoutDesc.rootConstants = &rootConstant;
      pipelineLayoutDesc.rootSamplerCount = 1;
      pipelineLayoutDesc.rootSamplers = &rootSampler;
      pipelineLayoutDesc.descriptorSetCount = descriptorSetDescs.size();
      pipelineLayoutDesc.descriptorSets = descriptorSetDescs.data();
      pipelineLayoutDesc.shaderStages = RHI::StageBits::VertexShader | RHI::StageBits::FragmentShader;

      coreInterface.createPipelineLayout(*renderingDevice, pipelineLayoutDesc, pipelineLayout);
    }

    // Pipeline
    const RHI::DeviceInfo &deviceInfo = coreInterface.getDeviceInfo(*renderingDevice);
    // utils::ShaderCodeStorage shaderCodeStorage;
    {
      RHI::VertexStreamInfo vertexStreamDesc = {};
      vertexStreamDesc.bindingSlot = 0;
      vertexStreamDesc.stride = deviceInfo.features.extendedDynamicState ? 0 : sizeof(Vertex);

      RHI::VertexAttributeInfo vertexAttributeDesc[2] = {};
      {
        vertexAttributeDesc[0].format = RHI::DataFormat::RG32_SFLOAT;
        vertexAttributeDesc[0].streamIndex = 0;
        vertexAttributeDesc[0].offset = offsetof(Vertex, position);
        vertexAttributeDesc[0].d3d = {"POSITION", 0};
        vertexAttributeDesc[0].vulkan.location = {0};

        vertexAttributeDesc[1].format = RHI::DataFormat::RG32_SFLOAT;
        vertexAttributeDesc[1].streamIndex = 0;
        vertexAttributeDesc[1].offset = offsetof(Vertex, uv);
        vertexAttributeDesc[1].d3d = {"TEXCOORD", 0};
        vertexAttributeDesc[1].vulkan.location = {1};
      }

      RHI::VertexInputInfo vertexInputDesc = {};
      vertexInputDesc.attributes = vertexAttributeDesc;
      // vertexInputDesc.attributeCount = (uint8_t)helper::GetCountOf(vertexAttributeDesc);
      vertexInputDesc.streams = &vertexStreamDesc;
      vertexInputDesc.streamCount = 1;

      RHI::InputAssemblyInfo inputAssemblyDesc = {};
      inputAssemblyDesc.topology = RHI::Topology::TriangleList;

      RHI::RasterizationInfo rasterizationDesc = {};
      rasterizationDesc.fillMode = RHI::FillMode::Solid;
      rasterizationDesc.cullMode = RHI::CullMode::None;

      RHI::ColorAttachmentInfo colorAttachmentDesc = {};
      // colorAttachmentDesc.format = swapChainFormat;
      colorAttachmentDesc.colorWriteMask = RHI::ColorWriteBits::RGBA;
      colorAttachmentDesc.blendEnabled = true;
      colorAttachmentDesc.colorBlend = {
        RHI::BlendFactor::SrcAlpha, RHI::BlendFactor::OneMinusSrcAlpha, RHI::BlendOp::Add
      };

      RHI::OutputMergerInfo outputMergerDesc = {};
      outputMergerDesc.colors = &colorAttachmentDesc;
      outputMergerDesc.colorCount = 1;

      // RHI::ShaderInfo shaderStages[] = {
      //     utils::LoadShader(deviceDesc.graphicsAPI, "TriangleFlexibleMultiview.vs", shaderCodeStorage),
      //     utils::LoadShader(deviceDesc.graphicsAPI, "Triangle.fs", shaderCodeStorage),
      // };

      RHI::GraphicsPipelineInfo graphicsPipelineDesc = {};
      graphicsPipelineDesc.pipelineLayout = pipelineLayout;
      graphicsPipelineDesc.vertexInput = &vertexInputDesc;
      graphicsPipelineDesc.inputAssembly = inputAssemblyDesc;
      graphicsPipelineDesc.rasterization = rasterizationDesc;
      graphicsPipelineDesc.outputMerger = outputMergerDesc;
      // graphicsPipelineDesc.shaders = shaderStages;
      // graphicsPipelineDesc.shaderCount = helper::GetCountOf(shaderStages);
      graphicsPipelineDesc.cache = pipelineCache;

      // double t0 = m_Timer.GetTimeStamp();
      (coreInterface.createGraphicsPipeline(*renderingDevice, graphicsPipelineDesc, pipeline));
      // double t1 = m_Timer.GetTimeStamp();
      // printf("CreateGraphicsPipeline (main) took %.3f ms\n", t1 - t0);

      // Multiview
      if (deviceInfo.features.flexibleMultiview) {
        graphicsPipelineDesc.outputMerger.viewMask = VIEW_MASK;
        graphicsPipelineDesc.outputMerger.multiview = RHI::Multiview::Flexible;

        // double t2 = m_Timer.GetTimeStamp();
        (coreInterface.createGraphicsPipeline(*renderingDevice, graphicsPipelineDesc, pipelineMultiview));
        // double t3 = m_Timer.GetTimeStamp();
        // printf("CreateGraphicsPipeline (multiview) took %.3f ms\n", t3 - t2);
      }
    }
  }

  Renderer::~Renderer() {
    if (renderingDevice) {
      coreInterface.deviceWaitIdle(renderingDevice);

      coreInterface.destroyPipeline(pipeline);
      coreInterface.destroyPipeline(pipelineMultiview);
      coreInterface.destroyPipelineLayout(pipelineLayout);
      // Pipeline cache
      if (pipelineCache) {
        uint64_t size = 0;
        // NRI_ABORT_ON_FAILURE(NRI.GetPipelineCacheData(*m_PipelineCache, nullptr, size));
        //
        // if (size > 0) {
        //   std::vector<uint8_t> blob(size);
        //   NRI_ABORT_ON_FAILURE(NRI.GetPipelineCacheData(*m_PipelineCache, blob.data(), size));
        //
        //   std::ofstream(PSO_CACHE_PATH, std::ios::binary).write((const char*)blob.data(), (std::streamsize)size);
        //   printf("Pipeline cache: saved %" PRIu64 " bytes to '%s'\n", size, PSO_CACHE_PATH);
        // }

        coreInterface.destroyPipelineCache(pipelineCache);
      }

      swapChainInterface.destroySwapChain(*renderingDevice, swapChain);

      RHI::destroyDevice(renderingDevice);
    }
  }

  void Renderer::createSwapChain(const Window *window, uint32_t width, uint32_t height) {
    coreInterface.getQueue(*renderingDevice, RHI::QueueType::Graphics, 0, graphicsQueue);

    RHI::SwapChainInfo swapChainInfo = {};

    swapChainInfo.flags |= RHI::SwapChainBits::AllowTearing;
    if (window->options.vsync)
      swapChainInfo.flags |= RHI::SwapChainBits::VSync;
    else
      swapChainInfo.flags |= RHI::SwapChainBits::None;

    swapChainInfo.windowHandle = window->getWindowHandle();
    swapChainInfo.presentQueue = graphicsQueue;
    swapChainInfo.format = RHI::SwapChainFormat::BT709_G22_8BIT;
    swapChainInfo.width = width;
    swapChainInfo.height = height;
    swapChainInfo.textureCount = getOptimalSwapChainTextureNum();
    swapChainInfo.queuedFrameCount = getQueuedFrameCount();

    swapChainInterface.createSwapChain(*renderingDevice, swapChainInfo, swapChain);

    //   uint32_t swapChainTextureNum;
    //   RHI::Texture* const* swapChainTextures = NRI.GetSwapChainTextures(*m_SwapChain, swapChainTextureNum);
    //
    //   swapChainFormat = NRI.GetTextureDesc(*swapChainTextures[0]).format;
    //
    //   for (uint32_t i = 0; i < swapChainTextureNum; i++) {
    //       RHI::TextureViewDesc textureViewDesc = {swapChainTextures[i], RHI::TextureView::COLOR_ATTACHMENT, swapChainFormat};
    //
    //       RHI::Descriptor* colorAttachment = nullptr;
    //       NRI_ABORT_ON_FAILURE(NRI.CreateTextureView(textureViewDesc, colorAttachment));
    //
    //       RHI::Fence* acquireSemaphore = nullptr;
    //       NRI_ABORT_ON_FAILURE(NRI.CreateFence(*m_Device, RHI::SWAPCHAIN_SEMAPHORE, acquireSemaphore));
    //
    //       RHI::Fence* releaseSemaphore = nullptr;
    //       NRI_ABORT_ON_FAILURE(NRI.CreateFence(*m_Device, RHI::SWAPCHAIN_SEMAPHORE, releaseSemaphore));
    //
    //       SwapChainTexture& swapChainTexture = m_SwapChainTextures.emplace_back();
    //
    //       swapChainTexture = {};
    //       swapChainTexture.acquireSemaphore = acquireSemaphore;
    //       swapChainTexture.releaseSemaphore = releaseSemaphore;
    //       swapChainTexture.texture = swapChainTextures[i];
    //       swapChainTexture.colorAttachment = colorAttachment;
    //       swapChainTexture.attachmentFormat = swapChainFormat;
    //   }
  }
}
