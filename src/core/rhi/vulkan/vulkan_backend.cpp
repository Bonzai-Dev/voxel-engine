#include "vulkan_backend.hpp"
#include "vulkan_pipeline_cache.hpp"
#include "vulkan_pipeline_layout.hpp"

using namespace Core::RHI;

namespace {
  template <typename Implementation, typename Interface, typename... Args>
  ENGINE_FORCE_INLINE Result createImplementation(Device &device, Interface *&entity, const Args &... args) {
    Implementation *impl = new Implementation(dynamic_cast<VulkanDevice&>(device));
    Result result = impl->create(args...);

    if (result != Result::Success) {
      delete impl;
      entity = nullptr;
    }
    else
      entity = (Interface*)impl;

    return result;
  }

  template <typename T>
  ENGINE_FORCE_INLINE void destroyImplementation(T *implementation) {
    if (implementation) {
      T *impl = (T*)(implementation);
      delete impl;
      impl = nullptr;
    }
  }
}

//============================================================================================================================================================================================
#pragma region[  Swap Chain  ]
namespace {
  Result createSwapChain(Device &device, const SwapChainInfo &swapChainInfo, SwapChain *&swapChain) {
    return createImplementation<VulkanSwapChain>(device, swapChain, swapChainInfo);
  }

  void destroySwapChain(Device &device, SwapChain *swapChain) {
    destroyImplementation<VulkanSwapChain>((VulkanSwapChain*)swapChain);
  }
}

Result VulkanDevice::loadInterface(Device &device, SwapChainInterface &swapChainInterface) {
  swapChainInterface.createSwapChain = ::createSwapChain;
  swapChainInterface.destroySwapChain = ::destroySwapChain;
  return Result::Success;
}
#pragma endregion

//============================================================================================================================================================================================
#pragma region[  Core  ]
namespace {
  Result getQueue(Device &device, QueueType queueType, uint32_t queueIndex, Queue *&queue) {
    return dynamic_cast<VulkanDevice&>(device).getQueue(queueType, queueIndex, queue);
  }

  Result deviceWaitIdle(Device *device) {
    if (!device)
      return Result::Success;

    return dynamic_cast<VulkanDevice*>(device)->waitIdle();
  }

  Result createPipelineLayout(
    Device &device,
    const PipelineLayoutInfo &pipelineLayoutDesc,
    PipelineLayout *&pipelineLayout
  ) {
    return createImplementation<VulkanPipelineLayout>(device, pipelineLayout, pipelineLayoutDesc);
  }

  Result createGraphicsPipeline(Device &device, const GraphicsPipelineInfo &pipelineInfo, Pipeline *&pipeline) {
    return createImplementation<VulkanPipeline>(device, pipeline, pipelineInfo);
  }

  const DeviceInfo & getDeviceInfo(const Device &device) {
    return ((VulkanDevice&)device).getInfo();
  }

  void destroyPipeline(Pipeline *pipeline) {
    destroyImplementation<VulkanPipeline>((VulkanPipeline*)pipeline);
  }

  void destroyPipelineLayout(PipelineLayout *pipelineLayout) {
    destroyImplementation<VulkanPipelineLayout>((VulkanPipelineLayout*)pipelineLayout);
  }

  void destroyPipelineCache(PipelineCache *pipelineCache) {
    destroyImplementation<VulkanPipelineCache>((VulkanPipelineCache*)pipelineCache);
  }
}

Result VulkanDevice::loadInterface(Device &device, CoreInterface &coreInterface) {
  coreInterface.getDeviceInfo = ::getDeviceInfo;
  coreInterface.getQueue = ::getQueue;
  coreInterface.deviceWaitIdle = ::deviceWaitIdle;

  coreInterface.createPipelineLayout = ::createPipelineLayout;
  coreInterface.createGraphicsPipeline = ::createGraphicsPipeline;

  coreInterface.destroyPipelineLayout = ::destroyPipelineLayout;
  coreInterface.destroyPipeline = ::destroyPipeline;
  coreInterface.destroyPipelineCache = ::destroyPipelineCache;
  return Result::Success;
}
#pragma endregion

namespace Core::RHI {
  Result createVulkanDevice(const DeviceCreateInfo &createInfo, Device *&device) {
    VulkanDevice *impl = new VulkanDevice(createInfo.callbackInterface);
    Result result = impl->create(createInfo);

    if (result != Result::Success) {
      delete impl;
      device = nullptr;
    }
    else {
      device = (Device*)impl;
    }

    return result;
  }
}
