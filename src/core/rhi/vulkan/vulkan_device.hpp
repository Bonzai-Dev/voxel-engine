#pragma once
#include <memory>
#include <vector>
#include <volk.h>
#include <vk_mem_alloc.h>
#include <core/rhi/rhi.hpp>
#include "core/rhi/stl/lock.hpp"

namespace Core::RHI {
  class VulkanQueue;

  struct RenderPassAttachmentInfo {
    bool operator==(const RenderPassAttachmentInfo &other) const {
      return format == other.format
        && sampleNum == other.sampleNum
        && loadOp == other.loadOp
        && storeOp == other.storeOp
        && stencilLoadOp == other.stencilLoadOp
        && stencilStoreOp == other.stencilStoreOp
        && layout == other.layout;
    }

    VkFormat format = VK_FORMAT_UNDEFINED;
    VkSampleCountFlagBits sampleNum = VK_SAMPLE_COUNT_1_BIT;
    VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    VkAttachmentLoadOp stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    VkAttachmentStoreOp stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
  };

  class RenderPassInfo {
    public:
      RenderPassInfo() = default;

      bool operator==(const RenderPassInfo &other) const {
        return viewMask == other.viewMask
          && hasDepth == other.hasDepth
          && hasStencil == other.hasStencil
          && hasDepthResolve == other.hasDepthResolve
          && hasStencilResolve == other.hasStencilResolve
          && hasShadingRate == other.hasShadingRate
          && depthResolveMode == other.depthResolveMode
          && stencilResolveMode == other.stencilResolveMode
          && colors == other.colors
          && colorResolves == other.colorResolves
          && inputAttachmentIndices == other.inputAttachmentIndices
          && depth == other.depth
          && stencil == other.stencil
          && depthResolve == other.depthResolve
          && stencilResolve == other.stencilResolve
          && shadingRate == other.shadingRate;
      }

      std::vector<RenderPassAttachmentInfo> colors;
      std::vector<RenderPassAttachmentInfo> colorResolves;
      std::vector<uint32_t> inputAttachmentIndices;
      RenderPassAttachmentInfo depth = {};
      RenderPassAttachmentInfo stencil = {};
      RenderPassAttachmentInfo depthResolve = {};
      RenderPassAttachmentInfo stencilResolve = {};
      RenderPassAttachmentInfo shadingRate = {};
      VkResolveModeFlagBits depthResolveMode = VK_RESOLVE_MODE_NONE;
      VkResolveModeFlagBits stencilResolveMode = VK_RESOLVE_MODE_NONE;
      uint32_t viewMask = 0;
      bool hasDepth = false;
      bool hasStencil = false;
      bool hasDepthResolve = false;
      bool hasStencilResolve = false;
      bool hasShadingRate = false;
  };

  struct FramebufferInfo {
    bool operator==(const FramebufferInfo &other) const {
      return renderPass == other.renderPass && width == other.width && height == other.height && layerNum == other.
        layerNum && attachments == other.attachments;
    }

    std::vector<VkImageView> attachments;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t layerNum = 0;
  };

  struct RenderPassCacheEntry {
    RenderPassInfo info;
    VkRenderPass handle = VK_NULL_HANDLE;
  };

  struct FramebufferCacheEntry {
    FramebufferInfo info;
    VkFramebuffer handle = VK_NULL_HANDLE;
  };

  struct VulkanDeviceFeatures {
    VkBool32 maintenance4 = VK_FALSE;
    VkBool32 maintenance5 = VK_FALSE;
    VkBool32 maintenance6 = VK_FALSE;
    VkBool32 maintenance7 = VK_FALSE;
    VkBool32 maintenance8 = VK_FALSE;
    VkBool32 maintenance9 = VK_FALSE;
    VkBool32 maintenance10 = VK_FALSE;
    VkBool32 deviceAddress = VK_FALSE;
    VkBool32 dynamicRendering = VK_FALSE;
    VkBool32 copyCommands2 = VK_FALSE;
    VkBool32 swapChainMutableFormat = VK_FALSE;
    VkBool32 presentId = VK_FALSE;
    VkBool32 memoryPriority = VK_FALSE;
    VkBool32 memoryBudget = VK_FALSE;
    VkBool32 imageSlicedView = VK_FALSE;
    VkBool32 customBorderColor = VK_FALSE;
    VkBool32 robustness = VK_FALSE;
    VkBool32 robustness2 = VK_FALSE;
    VkBool32 pipelineRobustness = VK_FALSE;
    VkBool32 swapChainMaintenance1 = VK_FALSE;
    VkBool32 fifoLatestReady = VK_FALSE;
    VkBool32 unifiedImageLayoutsVideo = VK_FALSE;
  };

  struct VulkanMemoryTypeInfo {
    uint16_t index;
    MemoryLocation location;
    bool mustBeDedicated;
  };

  class VulkanDevice final: public Device {
    public:
      VulkanDevice(const CallbackInterface &callbacks);

      ~VulkanDevice() override;

      inline operator VkDevice() const {
        return device;
      }

      inline operator VkPhysicalDevice() const {
        return physicalDevice;
      }

      inline operator VkInstance() const {
        return instance;
      }

      bool extensionSupported(
        const char *extension,
        const std::vector<VkExtensionProperties> &supportedExtensions
      ) const;
      bool extensionSupported(const char *extension, const std::vector<const char*> &supportedExtensions) const;

      Result create(const DeviceCreateInfo &createInfo);

      VkRenderPass getOrCreateRenderPass(const RenderPassInfo &info);
      // inline VkFramebuffer getOrCreateFramebuffer(const FramebufferInfo& info);

      bool getMemoryInfo(
        MemoryLocation memoryLocation,
        const VkMemoryRequirements &memoryRequirements,
        const VkMemoryDedicatedRequirements &memoryDedicatedRequirements,
        MemoryInfo &memoryInfo
      ) const;
      inline FormatSupportBits getFormatSupport(DataFormat format) const;
      inline const DeviceInfo &getInfo() const { return deviceInfo; }
      inline VmaAllocator_T *getVma() const { return vmaAllocator; }
      VkAllocationCallbacks *getAllocationCallbacks() const { return allocationCallbacks; }
      const VulkanDeviceFeatures &getVulkanFeatures() const { return vulkanFeatures; }
      bool memoryZeroInitializationEnabled() const { return isMemoryZeroInitializationEnabled; }

      Result waitIdle();
      Result getQueue(QueueType type, uint32_t queueIndex, Queue *&queue);

      // Result createSwapChain(const SwapChainInfo &swapChainInfo, SwapChain *&swapChain);
      // void destroySwapChain(SwapChain *swapChain);

    private:
      Result loadInterface(Device &device, CoreInterface &coreInterface) override;

      Result loadInterface(Device &device, SwapChainInterface &swapChainInterface) override;

      Result createInstance(bool validationLayerEnabled, const std::vector<const char*> &enabledExtensions);

      void loadInstanceExtensions(std::vector<const char*> &enabledExtensions);

      void loadDeviceExtensions(std::vector<const char*> &enabledDeviceExtensions, bool disableRayTracing);

      ENGINE_FORCE_INLINE void addExtension(
        const char *extension,
        std::vector<const char*> &enabledExtensions,
        const std::vector<VkExtensionProperties> &supportedExtensions
      ) const;

      uint8_t majorVersion = 1;
      uint8_t minorVersion = 0;

      VkInstance instance = VK_NULL_HANDLE;
      VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
      VkDevice device = VK_NULL_HANDLE;
      VkAllocationCallbacks *allocationCallbacks = nullptr;

      bool isMemoryZeroInitializationEnabled = false;

      VkPhysicalDeviceMemoryProperties memoryProperties = {};
      std::vector<RenderPassCacheEntry> renderPasses;
      std::vector<FramebufferCacheEntry> framebuffers;

      VulkanDeviceFeatures vulkanFeatures = {};
      DeviceInfo deviceInfo = {};

      std::array<uint32_t, static_cast<size_t>(QueueType::Count)> activeQueueFamilyIndices = {};
      uint32_t activeFamilyIndicesCount = 0;
      std::array<std::vector<std::unique_ptr<VulkanQueue>>, static_cast<std::size_t>(QueueType::Count)> queueFamilies;

      VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
      VmaAllocator_T *vmaAllocator = nullptr;

      Lock lock;
  };
}
