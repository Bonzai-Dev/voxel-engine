#pragma once

namespace Core::RHI {
  class VulkanQueue: public Queue {
    public:
      VulkanQueue(VulkanDevice &device);

      ~VulkanQueue() = default;

      inline operator VkQueue() const {
        return queue;
      }

      Result create(QueueType type, uint32_t familyIndex, VkQueue queue);

      inline VulkanDevice &getDevice() const {
        return device;
      }

      inline uint32_t getFamilyIndex() const {
        return familyIndex;
      }

      inline QueueType getType() const {
        return type;
      }

      inline Lock &getLock() {
        return lock;
      }

      Result waitIdle();

    private:
      VulkanDevice &device;
      VkQueue queue = VK_NULL_HANDLE;
      uint32_t familyIndex = invalidQueueFamilyIndex;
      QueueType type = QueueType::Graphics;
      Lock lock;
  };
}