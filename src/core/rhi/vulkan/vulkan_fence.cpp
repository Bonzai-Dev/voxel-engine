#include <cstdint>
#include <core/math/math.hpp>
#include "vulkan_backend.hpp"

namespace Core::RHI {
  VulkanFence::VulkanFence(VulkanDevice &device): device(device) {}

  VulkanFence::~VulkanFence() {
    if (semaphore)
      vkDestroySemaphore(device, semaphore, &device.vulkanAllocationCallbacks);
  }

  void VulkanFence::wait(uint64_t value) {
    VkSemaphoreWaitInfo semaphoreWaitInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO};
    semaphoreWaitInfo.semaphoreCount = 1;
    semaphoreWaitInfo.pSemaphores = &semaphore;
    semaphoreWaitInfo.pValues = &value;

    vkWaitSemaphores(device, &semaphoreWaitInfo, Math::Units::msToUs(fenceTimeout));
  }

  Result VulkanFence::create(uint64_t initialValue) {
    VkSemaphoreTypeCreateInfo semaphoreTypeCreateInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO};
    // semaphoreTypeCreateInfo.semaphoreType = initialValue == SWAPCHAIN_SEMAPHORE ? VK_SEMAPHORE_TYPE_BINARY : VK_SEMAPHORE_TYPE_TIMELINE;
    // semaphoreTypeCreateInfo.initialValue = initialValue == SWAPCHAIN_SEMAPHORE ? 0 : initialValue;

    VkSemaphoreCreateInfo semaphoreCreateInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    semaphoreCreateInfo.pNext = &semaphoreTypeCreateInfo;

    VULKAN_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, &device.vulkanAllocationCallbacks, &semaphore));

    return Result::Success;
  }
}
