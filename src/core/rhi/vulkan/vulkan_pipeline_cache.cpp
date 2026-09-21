#include "vulkan_backend.hpp"
#include "vulkan_pipeline_cache.hpp"
#include "vulkan_device.hpp"

namespace Core::RHI {
  VulkanPipelineCache::~VulkanPipelineCache() {
    if (pipelineCache)
      vkDestroyPipelineCache(device, pipelineCache, device.getAllocationCallbacks());
  }

  Result VulkanPipelineCache::create(const PipelineCacheInfo& pipelineCacheDesc) {
    if (!device.getInfo().features.pipelineCache)
      return Result::Success;

    VkPipelineCacheCreateInfo info = {VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO};
    info.initialDataSize = (size_t)pipelineCacheDesc.size;
    info.pInitialData = pipelineCacheDesc.data;

    VULKAN_CHECK(vkCreatePipelineCache(device, &info, device.getAllocationCallbacks(), &pipelineCache));

    size_t size = 0;
    VULKAN_CHECK(vkGetPipelineCacheData(device, pipelineCache, &size, nullptr));

    // VK returns "SUCCESS" for any variant of stale/incompatible data, try to guess...
    return size < info.initialDataSize ? Result::OutOfDate : Result::Success;
  }

  Result VulkanPipelineCache::getData(void* dst, uint64_t& size) const {
    if (!pipelineCache) {
      size = 0;
      return Result::Success;
    }

    // Theoretically may be smaller than needed to fit the entire cache...
    size_t vkSize = (size_t)size;

    // ...and even if "VK_INCOMPLETE" is returned, we consider it a "SUCCESS"
    VULKAN_CHECK(vkGetPipelineCacheData(device, pipelineCache, &vkSize, dst));

    size = vkSize;

    return Result::Success;
  }

}
