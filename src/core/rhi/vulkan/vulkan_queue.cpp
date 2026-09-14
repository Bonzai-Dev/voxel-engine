#include "vulkan_backend.hpp"

namespace Core::RHI {
  VulkanQueue::VulkanQueue(VulkanDevice &device): device(device) {
  }

  Result VulkanQueue::create(QueueType type, uint32_t familyIndex, VkQueue queue) {
    this->type = type;
    this->familyIndex = familyIndex;
    this->queue = queue;
    return Result::Success;
  }

  Result VulkanQueue::waitIdle() {
    ExclusiveScope lock(this->lock);
    VULKAN_CHECK(vkQueueWaitIdle(queue));
    return Result::Success;
  }
}