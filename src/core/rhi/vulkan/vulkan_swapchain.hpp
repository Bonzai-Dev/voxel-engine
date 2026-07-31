#pragma once
#include <SDL3/SDL_video.h>
#include "vulkan_context.hpp"
#include "volk.h"

namespace Core::Graphics {
  class VulkanSwapChain {
    public:
      VulkanSwapChain(
        const VkInstance &instance,
        RefCountedPtr<VulkanDevice> device,
        SDL_Window * window,
        std::uint32_t width,
        std::uint32_t height
      );

      VkSurfaceFormatKHR getSurfaceFormat() const { return surfaceFormat; }

      ~VulkanSwapChain() = default;

      void destroy();

      void present(VkPipeline pipeline, VkPipelineLayout pipelineLayout) const;

    private:
      std::uint32_t acquireNextImage() const;

      SDL_Window * const window = nullptr;

      const VkInstance &instance;
      RefCountedPtr<VulkanDevice> device;

      VkSurfaceKHR surface = VK_NULL_HANDLE;
      VkSwapchainKHR swapChain = VK_NULL_HANDLE;
      VkSurfaceCapabilitiesKHR surfaceCapabilities {};
      VkSurfaceFormatKHR surfaceFormat {};

      struct SwapChainCommandBuffer {
        VkCommandPool commandPool = nullptr;
        VkCommandBuffer commandBuffer = nullptr;
      };

      std::vector<SwapChainCommandBuffer> commandBuffers;

      std::vector<VkSemaphore> imageAvailableSemaphores;
      std::vector<VkSemaphore> renderFinishedSemaphores;
      std::vector<VkFence> waitFences;

      std::vector<VkImage> images;
      std::vector<VkImageView> imageViews;

      mutable std::uint32_t currentFrameIndex = 0;
      mutable std::uint32_t imageIndex = 0;

      mutable bool flash = false;

      mutable VkPipeline pipeline = nullptr;
      mutable VkPipelineLayout pipelineLayout = nullptr;
  };
}
