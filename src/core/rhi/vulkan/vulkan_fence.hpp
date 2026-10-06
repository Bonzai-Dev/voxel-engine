#pragma once

namespace Core::RHI {
  class VulkanFence {
    public:
      VulkanFence(VulkanDevice &device): device(device) {}

      ~VulkanFence();

      inline operator VkSemaphore() const {
        return semaphore;
      }

      inline VulkanDevice &getDevice() const {
        return device;
      }

      Result create(uint64_t initialValue);

      uint64_t getFenceValue() const;

      void wait(uint64_t value) const;

    private:
      VulkanDevice &device;
      VkSemaphore semaphore = VK_NULL_HANDLE;
  };
}