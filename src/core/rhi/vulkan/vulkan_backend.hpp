#pragma once
#include <string>
#include <vector>
#include <volk.h>
#include <vk_mem_alloc.h>
#include <core/logger.hpp>
#include <core/rhi/rhi.hpp>
#include <core/rhi/extensions/swap_chain.hpp>
#include <core/rhi/extensions/ray_tracing.hpp>
#include "core/rhi/stl/lock.hpp"

#define VULKAN_CHECK(vulkanCall) \
  if (vulkanCall != VK_SUCCESS) { \
    std::string vkfunc = #vulkanCall; \
    vkfunc = vkfunc.substr(0, vkfunc.find('(')); \
    LOG_CORE_ERROR( \
      "{} failed with {} at {}:{}", \
      vkfunc, vulkanResultToString(vulkanCall), __FILE__, __LINE__ \
    ); \
  return vulkanResultToResult(vulkanCall); \
} else {}

#define PNEXT_CHAIN_DECLARE(next) const void** _tail = (const void**)&next

#define PNEXT_CHAIN_SET(next) _tail = (const void**)&next

#define PNEXT_CHAIN_APPEND_STRUCT(desc) \
do { \
  *_tail = &(desc); \
  _tail = (const void**)&(desc).pNext; \
} while (0)

#include "vulkan_conversion.hpp"
#include "vulkan_device.hpp"
#include "vulkan_texture.hpp"
#include "vulkan_fence.hpp"
#include "vulkan_pipeline.hpp"
#include "vulkan_swapchain.hpp"
#include "vulkan_queue.hpp"

namespace Core::RHI {
  enum class Result: int8_t;

  constexpr uint32_t unusedRenderPassAttachment = uint32_t(-1);

  inline void setRenderPassInputAttachmentIndex(std::vector<uint32_t>& inputAttachmentIndices, uint32_t index) {
    while (inputAttachmentIndices.size() <= index)
      inputAttachmentIndices.push_back(unusedRenderPassAttachment);

    inputAttachmentIndices[index] = index;
  }

  inline bool hasRenderPassInputAttachmentIndex(const std::vector<uint32_t>& inputAttachmentIndices, uint32_t index) {
    return index < inputAttachmentIndices.size() && inputAttachmentIndices[index] != unusedRenderPassAttachment;
  }

  Result createVulkanDevice(const DeviceCreateInfo &createInfo, Device *&device);
}
