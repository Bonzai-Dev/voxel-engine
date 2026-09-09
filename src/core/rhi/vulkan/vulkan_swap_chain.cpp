#include <SDL3/SDL_vulkan.h>
#include <core/rhi/rhi.hpp>
#include "vulkan_backend.hpp"

namespace {
  using namespace Core::RHI;

  constexpr uint64_t presentIndexBitNumber = 56ull;
  constexpr uint32_t presentModeMaxCount = 16;

  constexpr VkPresentGravityFlagBitsKHR GetGravity(Gravity gravity) {
    switch (gravity) {
      case Gravity::Centered:
        return VK_PRESENT_GRAVITY_CENTERED_BIT_KHR;
      case Gravity::Max:
        return VK_PRESENT_GRAVITY_MAX_BIT_KHR;
      default:
        return VK_PRESENT_GRAVITY_MIN_BIT_KHR;
    }
  }

  uint64_t getPresentIndex(uint64_t presentId) {
    return presentId & ((1ull << presentIndexBitNumber) - 1ull);
  }

  uint64_t getSwapChainId() {
    static uint64_t id = 0;
    return id >> presentIndexBitNumber;
  }
}

namespace Core::RHI {
  VulkanSwapChain::VulkanSwapChain(VulkanDevice &device): device(device), textures(device.stdAllocator) {
  }

  Result VulkanSwapChain::create(const SwapChainInfo &swapChainInfo) {
    presentQueue = (VulkanQueue*)(swapChainInfo.presentQueue);
    uint32_t familyIndex = presentQueue->getFamilyIndex();

    {
      SDL_Vulkan_CreateSurface(swapChainInfo.window, device, &device.vulkanAllocationCallbacks, &surface);
      VkBool32 supported = VK_FALSE;
      VULKAN_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(device, familyIndex, surface, &supported));

      if (!supported) {
        LOG_CORE_CRITICAL("Window surface is not supported");
        return Result::Failure;
      }
    }

    // Low latency
    std::array<VkPresentModeKHR, presentModeMaxCount> lowLatencyPresentModes = {};

    VkLatencySurfaceCapabilitiesNV latencySurfaceCaps = {VK_STRUCTURE_TYPE_LATENCY_SURFACE_CAPABILITIES_NV};
    latencySurfaceCaps.presentModeCount = (uint32_t)lowLatencyPresentModes.size();
    latencySurfaceCaps.pPresentModes = lowLatencyPresentModes.data();

    bool allowLowLatency = device.getDeviceInfo().features.lowLatency &&
      (swapChainInfo.flags &SwapChainBits::AllowLowLatency);
    if (allowLowLatency) {
      VkPhysicalDeviceSurfaceInfo2KHR surfaceInfo = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR};
      surfaceInfo.surface = surface;

      VkSurfaceCapabilities2KHR surfaceCaps2 = {VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR};

      PNEXT_CHAIN_DECLARE(surfaceCaps2.pNext);
      PNEXT_CHAIN_APPEND_STRUCT(latencySurfaceCaps);

      VkResult vkResult = vkGetPhysicalDeviceSurfaceCapabilities2KHR(device, &surfaceInfo, &surfaceCaps2);
      // NRI_RETURN_ON_BAD_VKRESULT(&device, vkResult, "vkGetPhysicalDeviceSurfaceCapabilities2KHR");
    }

    // Surface format
    VkSurfaceFormat2KHR surfaceFormat = {VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR};
    {
      VkPhysicalDeviceSurfaceInfo2KHR surfaceInfo = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR};
      surfaceInfo.surface = surface;

      uint32_t formatNum = 0;
      VkResult vkResult = vkGetPhysicalDeviceSurfaceFormats2KHR(device, &surfaceInfo, &formatNum, nullptr);
      // NRI_RETURN_ON_BAD_VKRESULT(&device, vkResult, "vkGetPhysicalDeviceSurfaceFormats2KHR");

      Scratch<VkSurfaceFormat2KHR> surfaceFormats = NRI_ALLOCATE_SCRATCH(device, VkSurfaceFormat2KHR, formatNum);
      for (uint32_t i = 0; i < formatNum; i++)
        surfaceFormats[i] = {VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR};

      vkResult = vkGetPhysicalDeviceSurfaceFormats2KHR(device, &surfaceInfo, &formatNum, surfaceFormats);
      // NRI_RETURN_ON_BAD_VKRESULT(&device, vkResult, "vkGetPhysicalDeviceSurfaceFormats2KHR");

      auto priority_BT709_G22_16BIT = [](const VkSurfaceFormat2KHR &s) -> uint32_t {
        if (s.surfaceFormat.format != VK_FORMAT_R16G16B16A16_SFLOAT)
          return 0;

        uint32_t priority = s.surfaceFormat.colorSpace == VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT ? 20 : 10;

        return priority;
      };

      auto priority_BT709_G22_8BIT = [](const VkSurfaceFormat2KHR &s) -> uint32_t {
        // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/vkGetPhysicalDeviceSurfaceFormatsKHR.html
        // There is always a corresponding UNORM, SRGB just need to consider UNORM
        uint32_t priority = 0;
        if (s.surfaceFormat.format == VK_FORMAT_R8G8B8A8_UNORM)
          priority = 4;
        else if (s.surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM)
          priority = 3;
        else if (s.surfaceFormat.format == VK_FORMAT_R8G8B8A8_SRGB)
          priority = 2;
        else if (s.surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB)
          priority = 1;
        else
          return 0;

        priority += s.surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR ? 10 : 0;

        return priority;
      };

      auto priority_BT709_G22_10BIT = [](const VkSurfaceFormat2KHR &s) -> uint32_t {
        uint32_t priority = 0;
        if (s.surfaceFormat.format == VK_FORMAT_A2B10G10R10_UNORM_PACK32)
          priority = 1;
        else
          return 0;

        priority += s.surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR ? 10 : 0;

        return priority;
      };

      auto priority_BT2020_G2084_10BIT = [](const VkSurfaceFormat2KHR &s) -> uint32_t {
        uint32_t priority = 0;
        if (s.surfaceFormat.format == VK_FORMAT_A2B10G10R10_UNORM_PACK32)
          priority = 1;
        else
          return 0;

        priority += s.surfaceFormat.colorSpace == VK_COLOR_SPACE_HDR10_ST2084_EXT ? 10 : 0;

        return priority;
      };

      switch (swapChainInfo.format) {
        case SwapChainFormat::BT709_G10_16BIT:
          std::sort(surfaceFormats + 0, surfaceFormats + formatNum,
                    [&](VkSurfaceFormat2KHR &a1, VkSurfaceFormat2KHR &b1) {
                      return priority_BT709_G22_16BIT(a1) > priority_BT709_G22_16BIT(b1);
                    });
          break;
        case SwapChainFormat::BT709_G22_8BIT:
          std::sort(surfaceFormats + 0, surfaceFormats + formatNum,
                    [&](VkSurfaceFormat2KHR &a1, VkSurfaceFormat2KHR &b1) {
                      return priority_BT709_G22_8BIT(a1) > priority_BT709_G22_8BIT(b1);
                    });
          break;
        case SwapChainFormat::BT709_G22_10BIT:
          std::sort(surfaceFormats + 0, surfaceFormats + formatNum,
                    [&](VkSurfaceFormat2KHR &a1, VkSurfaceFormat2KHR &b1) {
                      return priority_BT709_G22_10BIT(a1) > priority_BT709_G22_10BIT(b1);
                    });
          break;
        case SwapChainFormat::BT2020_G2084_10BIT:
          std::sort(surfaceFormats + 0, surfaceFormats + formatNum,
                    [&](VkSurfaceFormat2KHR &a1, VkSurfaceFormat2KHR &b1) {
                      return priority_BT2020_G2084_10BIT(a1) > priority_BT2020_G2084_10BIT(b1);
                    });
          break;
        default:
          ENGINE_ASSERT(false, "Unexpected swap chain format {}", swapChainFormatToString(swapChainInfo.format));
          break;
      }

      surfaceFormat = surfaceFormats[0];
    }

    /* Present modes:
        Capped frame rate            Vsync  Comments
            FIFO                       Y       Classic VSYNC, always supported
            FIFO_RELAXED              Y/N      FIFO with fallback to IMMEDIATE if framerate < monitor refresh rate

        Uncapped frame rate          Vsync  Comments
            IMMEDIATE                  N       Classic tearing, most likely supported
            MAILBOX                    Y       No tearing, but almost uncapped FPS
            FIFO_LATEST_READY          Y       Similar to MAILBOX, but offers lower latency
    */
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR; // the only one 100% supported (see spec)
    {
      uint32_t surfacePresentModeNum = presentModeMaxCount;
      std::array<VkPresentModeKHR, presentModeMaxCount> surfacePresentModes = {};

      VULKAN_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
        device, surface, &surfacePresentModeNum, surfacePresentModes.data()
      ));

      const std::array cappedModes = {
        (swapChainInfo.flags & SwapChainBits::AllowTearing)
          ? VK_PRESENT_MODE_FIFO_RELAXED_KHR
          : VK_PRESENT_MODE_FIFO_KHR,
        VK_PRESENT_MODE_FIFO_KHR, // guaranteed to be supported
      };

      const std::array uncappedModes = {
        (swapChainInfo.flags & SwapChainBits::AllowTearing)
          ? VK_PRESENT_MODE_IMMEDIATE_KHR
          : VK_PRESENT_MODE_MAILBOX_KHR,
        (swapChainInfo.flags & SwapChainBits::AllowTearing)
          ? VK_PRESENT_MODE_IMMEDIATE_KHR
          : (device.getSupportedFeatures().fifoLatestReady ? VK_PRESENT_MODE_FIFO_LATEST_READY_EXT : VK_PRESENT_MODE_FIFO_KHR),
        VK_PRESENT_MODE_FIFO_KHR, // guaranteed to be supported
      };

      const VkPresentModeKHR *wantedModes = uncappedModes.data();
      uint32_t wantedModeNum = uncappedModes.size();

      if (swapChainInfo.flags & SwapChainBits::VSync) {
        wantedModes = cappedModes.data();
        wantedModeNum = cappedModes.size();
      }

      const VkPresentModeKHR *availableModes = surfacePresentModes.data();
      uint32_t availableModeNum = surfacePresentModeNum;
      if (allowLowLatency) {
        availableModes = lowLatencyPresentModes.data();
        availableModeNum = latencySurfaceCaps.presentModeCount;
      }

      bool isFound = false;
      for (uint32_t j = 0; j < wantedModeNum && !isFound; j++) {
        for (uint32_t i = 0; i < availableModeNum && !isFound; i++) {
          if (wantedModes[j] == availableModes[i]) {
            presentMode = wantedModes[j];
            isFound = true;
          }
        }
      }
    }

    // Scaling mode and caps
    bool isScalingSupported = false;
    uint32_t textureNum = swapChainInfo.textureNum;
    {
      VkPhysicalDeviceSurfaceInfo2KHR surfaceInfo = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR};
      surfaceInfo.surface = surface;

      VkSurfacePresentModeKHR surfacePresentMode = {VK_STRUCTURE_TYPE_SURFACE_PRESENT_MODE_KHR};
      surfacePresentMode.presentMode = presentMode;

      VkSurfaceCapabilities2KHR surfaceCaps2 = {VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR};

      VkSurfacePresentScalingCapabilitiesKHR surfacePresentScalingCaps = {
        VK_STRUCTURE_TYPE_SURFACE_PRESENT_SCALING_CAPABILITIES_KHR
      };

      PNEXT_CHAIN_DECLARE(surfaceCaps2.pNext);
      if (device.getSupportedFeatures().swapChainMaintenance1)
        PNEXT_CHAIN_APPEND_STRUCT(surfacePresentScalingCaps);

      PNEXT_CHAIN_SET(surfaceInfo.pNext);
      if (device.getSupportedFeatures().swapChainMaintenance1)
        PNEXT_CHAIN_APPEND_STRUCT(surfacePresentMode);

      VULKAN_CHECK(vkGetPhysicalDeviceSurfaceCapabilities2KHR(device, &surfaceInfo, &surfaceCaps2));

      // Caps
      const VkSurfaceCapabilitiesKHR &surfaceCaps = surfaceCaps2.surfaceCapabilities;
      bool isWidthValid = swapChainInfo.width >= surfaceCaps.minImageExtent.width && swapChainInfo.width <= surfaceCaps.
        maxImageExtent.width;
      if (!isWidthValid) {
        LOG_CORE_ERROR(
          "Swap chain width is out of [{}, {}] range",
          surfaceCaps.minImageExtent.width, surfaceCaps.maxImageExtent.width
        );
        return Result::InvalidArgument;
      }

      bool isHeightValid = swapChainInfo.height >= surfaceCaps.minImageExtent.height && swapChainInfo.height <=
        surfaceCaps.maxImageExtent.height;

      if (!isHeightValid) {
        LOG_CORE_ERROR(
          "Swap chain height is out of [{}, {}] range",
          surfaceCaps.minImageExtent.height, surfaceCaps.maxImageExtent.height
        );
      }

      // Silently clamp "textureNum" to the supported range
      if (textureNum < surfaceCaps.minImageCount)
        textureNum = surfaceCaps.minImageCount;
      if (surfaceCaps.maxImageCount && textureNum > surfaceCaps.maxImageCount) // 0 - unlimited (see spec)
        textureNum = surfaceCaps.maxImageCount;

      if (textureNum != swapChainInfo.textureNum) {
        LOG_CORE_WARNING("Swap chain texture count of {} has been clamped to {}", swapChainInfo.textureNum, textureNum);
      }

      // TODO: that's the minimal check to detect scaling support
      if (surfacePresentScalingCaps.supportedPresentScaling != 0 && surfacePresentScalingCaps.supportedPresentGravityX
        != 0 && surfacePresentScalingCaps.supportedPresentGravityY != 0)
        isScalingSupported = true;
    }

    constexpr VkImageUsageFlags swapchainImageUsageFlags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT
      | VK_IMAGE_USAGE_TRANSFER_DST_BIT
      | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    {
      // Swap chain
      VkSwapchainCreateInfoKHR swapchainInfo = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
      swapchainInfo.flags = 0;
      swapchainInfo.surface = surface;
      swapchainInfo.minImageCount = textureNum;
      swapchainInfo.imageFormat = surfaceFormat.surfaceFormat.format;
      swapchainInfo.imageColorSpace = surfaceFormat.surfaceFormat.colorSpace;
      swapchainInfo.imageExtent = {swapChainInfo.width, swapChainInfo.height};
      swapchainInfo.imageArrayLayers = 1;
      swapchainInfo.imageUsage = swapchainImageUsageFlags;
      swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
      swapchainInfo.queueFamilyIndexCount = 1;
      swapchainInfo.pQueueFamilyIndices = &familyIndex;
      swapchainInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
      swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
      swapchainInfo.presentMode = presentMode;
      PNEXT_CHAIN_DECLARE(swapchainInfo.pNext);

      VkSwapchainPresentScalingCreateInfoKHR scalingInfo = {
        VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_SCALING_CREATE_INFO_KHR
      };
      if (isScalingSupported) {
        scalingInfo.scalingBehavior = swapChainInfo.scaling == Scaling::Stretch
                                        ? VK_PRESENT_SCALING_STRETCH_BIT_KHR
                                        : VK_PRESENT_SCALING_ONE_TO_ONE_BIT_KHR;
        scalingInfo.presentGravityX = GetGravity(swapChainInfo.gravityX);
        scalingInfo.presentGravityY = GetGravity(swapChainInfo.gravityY);
        PNEXT_CHAIN_APPEND_STRUCT(scalingInfo);
      }

      // Mutable formats
      VkFormat mutableFormats[2];
      uint32_t mutableFormatNum = 0;
      mutableFormats[mutableFormatNum++] = surfaceFormat.surfaceFormat.format;
      switch (surfaceFormat.surfaceFormat.format) {
        case VK_FORMAT_R8G8B8A8_UNORM:
          mutableFormats[mutableFormatNum++] = VK_FORMAT_R8G8B8A8_SRGB;
          break;
        case VK_FORMAT_R8G8B8A8_SRGB:
          mutableFormats[mutableFormatNum++] = VK_FORMAT_R8G8B8A8_UNORM;
          break;
        case VK_FORMAT_B8G8R8A8_UNORM:
          mutableFormats[mutableFormatNum++] = VK_FORMAT_B8G8R8A8_SRGB;
          break;
        case VK_FORMAT_B8G8R8A8_SRGB:
          mutableFormats[mutableFormatNum++] = VK_FORMAT_B8G8R8A8_UNORM;
          break;
        default:
          break;
      }

      VkImageFormatListCreateInfo imageFormatListCreateInfo = {VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO};
      imageFormatListCreateInfo.pViewFormats = mutableFormats;
      imageFormatListCreateInfo.viewFormatCount = mutableFormatNum;

      if (device.getSupportedFeatures().swapChainMutableFormat) {
        swapchainInfo.flags |= VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR;
        PNEXT_CHAIN_APPEND_STRUCT(imageFormatListCreateInfo);
      }

      // Low latency mode
      VkSwapchainLatencyCreateInfoNV latencyCreateInfo = {VK_STRUCTURE_TYPE_SWAPCHAIN_LATENCY_CREATE_INFO_NV};
      latencyCreateInfo.latencyModeEnable = allowLowLatency;

      if (allowLowLatency)
        PNEXT_CHAIN_APPEND_STRUCT(latencyCreateInfo);

      // Create
      VULKAN_CHECK(vkCreateSwapchainKHR(device, &swapchainInfo, &device.vulkanAllocationCallbacks, &swapChain));
    }

    {
      // Textures
      uint32_t imageNum = 0;
      vkGetSwapchainImagesKHR(device, swapChain, &imageNum, nullptr);

      Scratch<VkImage> imageHandles = NRI_ALLOCATE_SCRATCH(device, VkImage, imageNum);
      vkGetSwapchainImagesKHR(device, swapChain, &imageNum, imageHandles);

      textures.resize(imageNum);
      for (uint32_t i = 0; i < imageNum; i++) {
        // VulkanTextureInfo desc = {};
        // desc.vkImage = (VKNonDispatchableHandle)imageHandles[i];
        // desc.vkFormat = surfaceFormat.surfaceFormat.format;
        // desc.vkImageType = VK_IMAGE_TYPE_2D;
        // desc.vkImageUsageFlags = swapchainImageUsageFlags;
        // desc.width = swapChainInfo.width;
        // desc.height = swapChainInfo.height;
        // desc.depth = 1;
        // desc.mipNum = 1;
        // desc.layerNum = 1;
        // desc.sampleNum = 1;
        //
        // VulkanTexture *texture = allocate<VulkanTexture>(device.allocationCallbacks, device);
        // texture->create(desc);
        // textures[i] = texture;

        VkImage image = imageHandles[i];
        VkFormat imageFormat = surfaceFormat.surfaceFormat.format;
        VkImageType imageType = VK_IMAGE_TYPE_2D;
        VkImageUsageFlags imageUsageFlags = swapchainImageUsageFlags;
        uint32_t imageWidth = swapChainInfo.width;
        uint32_t imageHeight = swapChainInfo.height;
        uint32_t imageDepth = 1;
        uint32_t imageMipNum = 1;
        uint32_t imageLayerNum = 1;
        uint32_t imageSampleNum = 1;

        if (swapchainImageUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT)
          this->imageUsageFlags |= TextureUsageBits::ShaderResource;

        if (swapchainImageUsageFlags & VK_IMAGE_USAGE_STORAGE_BIT)
          this->imageUsageFlags |= TextureUsageBits::ShaderResourceStorage;

        if (swapchainImageUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
          this->imageUsageFlags |= TextureUsageBits::ColorAttachment;

        if (swapchainImageUsageFlags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
          this->imageUsageFlags |= TextureUsageBits::DepthStencilAttachment;

        if (swapchainImageUsageFlags & VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR)
          this->imageUsageFlags |= TextureUsageBits::ShadingRateAttachment;

        if (swapchainImageUsageFlags & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)
          this->imageUsageFlags |= TextureUsageBits::InputAttachment;
      }
    }

    // Latency fence
    if (allowLowLatency) {
      latencyFence = allocate<VulkanFence>(device.allocationCallbacks, device);
      latencyFence->create(0);
    }

    // Finalize
    windowHandle = swapChainInfo.window;
    presentId = getSwapChainId();

    flags = swapChainInfo.flags;
    if (!allowLowLatency)
      flags &= ~SwapChainBits::AllowLowLatency;
    if (!device.getDeviceInfo().features.waitableSwapChain)
      flags &= ~SwapChainBits::Waitable;

    return Result::Success;
  }

  Result VulkanSwapChain::acquireNextImage(VulkanFence &acquireSemaphore, uint32_t &textureIndex) {
    ExclusiveScope lock(presentQueue->getLock());

    // Acquire next image (signal)
    VkAcquireNextImageInfoKHR acquireInfo = {VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR};
    acquireInfo.swapchain = swapChain;
    // acquireInfo.timeout = MsToUs(NRI_TIMEOUT_PRESENT);
    acquireInfo.semaphore = acquireSemaphore;
    // acquireInfo.deviceMask = NODE_MASK;

    VULKAN_CHECK(vkAcquireNextImage2KHR(device, &acquireInfo, &this->textureIndex));

    textureIndex = this->textureIndex;

    return Result::Success;
  }
}
