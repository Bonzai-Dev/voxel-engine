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

      ~VulkanSwapChain() = default;

      void destroy();

    private:
      SDL_Window * const window = nullptr;

      const VkInstance &instance;
      RefCountedPtr<VulkanDevice> device;

      VkSurfaceKHR surface = VK_NULL_HANDLE;
      VkSwapchainKHR swapChain = VK_NULL_HANDLE;
      VkSurfaceCapabilitiesKHR surfaceCapabilities{};
      VkSurfaceFormatKHR surfaceFormat;

      std::vector<VkImage> images;
      std::vector<VkImageView> imageViews;
  };
}
