#pragma once

namespace Core::RHI {
  class VulkanFence {
    public:
      VulkanFence(VulkanDevice &device);

      ~VulkanFence();

      inline operator VkSemaphore() const {
        return semaphore;
      }

      Result create(uint64_t initialValue);

      uint64_t getFenceValue() const;

      void wait(uint64_t value) const;

    private:
      VulkanDevice &device;
      VkSemaphore semaphore = VK_NULL_HANDLE;
  };
}