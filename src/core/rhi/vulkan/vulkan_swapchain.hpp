#pragma once

namespace Core::RHI {
  class VulkanSwapChain final: public SwapChain {
    public:
      VulkanSwapChain(VulkanDevice &device);

      ~VulkanSwapChain();

      Result create(const SwapChainInfo &swapChainInfo);

      ENGINE_FORCE_INLINE Result acquireNextImage(VulkanFence &acquireSemaphore, uint32_t &textureIndex);

    private:
      VulkanDevice &device;
      std::vector<VulkanTexture*> textures;

      std::unique_ptr<VulkanFence> latencyFence = nullptr;
      VulkanQueue *presentQueue = nullptr;

      VkSwapchainKHR swapChain = VK_NULL_HANDLE;
      VkSurfaceKHR surface = VK_NULL_HANDLE;

      void *windowHandle = nullptr;
      uint64_t presentId = 0;
      uint32_t textureIndex = 0;
      SwapChainBits flags = SwapChainBits::None;
  };


}