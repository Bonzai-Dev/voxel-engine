#include "vulkan_backend.hpp"

namespace Core::RHI {
  VulkanQueue::VulkanQueue(VulkanDevice &device): device(device) {
  }

  VulkanQueue::~VulkanQueue() {
  }

  Result VulkanQueue::create(QueueType type, uint32_t familyIndex, VkQueue queue) {
    this->type = type;
    this->familyIndex = familyIndex;
    this->queue = queue;
    return Result::Success;
  }

  Result VulkanQueue::waitIdle() {

    return Result::Success;
  }
}