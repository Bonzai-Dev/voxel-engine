#pragma once

namespace Core::RHI {
  class VulkanTexture final: public Texture {
    public:
      VulkanTexture(VulkanDevice &device);

      ~VulkanTexture() override;

      void getMemoryInfo(MemoryLocation memoryLocation, MemoryInfo &memoryInfo) const;

      Result create(const TextureInfo &textureInfo);

    private:
      VulkanDevice &device;
      VmaAllocation vmaAllocation = nullptr;
      VkImage image = VK_NULL_HANDLE;
      TextureInfo info = {};
  };
}