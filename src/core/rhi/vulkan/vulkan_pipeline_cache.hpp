#pragma once
#include <core/rhi/rhi.hpp>
#include "volk.h"

namespace Core::RHI {
  class VulkanDevice;

  class VulkanPipelineCache {
    public:
      inline VulkanPipelineCache(VulkanDevice &device): device(device) {
      }

      inline operator VkPipelineCache() const {
        return pipelineCache;
      }

      inline VulkanDevice &getDevice() const {
        return device;
      }

      ~VulkanPipelineCache();

      Result create(const PipelineCacheInfo &pipelineCacheDesc);
      Result getData(void *dst, uint64_t &size) const;

    private:
      VulkanDevice &device;
      VkPipelineCache pipelineCache = VK_NULL_HANDLE;
  };
}
