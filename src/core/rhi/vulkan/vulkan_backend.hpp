#pragma once
#include <volk.h>
#include <vk_mem_alloc.h>
#include <core/rhi/rhi.hpp>
#include "core/rhi/extensions/swap_chain.hpp"
#include "core/rhi/stl/allocator.hpp"
#include "core/rhi/stl/lock.hpp"

#define VULKAN_CHECK(vulkanCall) \
  if (vulkanCall != VK_SUCCESS) { \
    std::string vkfunc = #vulkanCall; \
    vkfunc = vkfunc.substr(0, vkfunc.find('(')); \
    LOG_CORE_ERROR( \
      "{} {} {} {}(): failed, result = {}", \
      resultToString(vulkanResultToResult(vulkanCall)), \
      __FILE__, __LINE__, \
      vkfunc, vulkanResultToString(vulkanCall) \
    ); \
  return vulkanResultToResult(vulkanCall); \
} else {}

#define PNEXT_CHAIN_DECLARE(next) const void** _tail = (const void**)&next

#define PNEXT_CHAIN_SET(next) _tail = (const void**)&next

#define PNEXT_CHAIN_APPEND_STRUCT(desc) \
do { \
  *_tail = &(desc); \
  _tail = (const void**)&(desc).pNext; \
} while (0)

namespace Core::RHI {
  enum class Result: int8_t;

  class VulkanFence;
  class VulkanDevice;
  class VulkanTexture;
  class VulkanSwapChain;
  class VulkanQueue;

  // Each depth/stencil format is only compatible with itself in VK
  constexpr std::array<VkFormat, static_cast<size_t>(Format::Count)> vulkanFormats = {
    VK_FORMAT_UNDEFINED, // UNKNOWN
    VK_FORMAT_R8_UNORM, // R8_UNORM
    VK_FORMAT_R8_SNORM, // R8_SNORM
    VK_FORMAT_R8_UINT, // R8_UINT
    VK_FORMAT_R8_SINT, // R8_SINT
    VK_FORMAT_R8G8_UNORM, // RG8_UNORM
    VK_FORMAT_R8G8_SNORM, // RG8_SNORM
    VK_FORMAT_R8G8_UINT, // RG8_UINT
    VK_FORMAT_R8G8_SINT, // RG8_SINT
    VK_FORMAT_B8G8R8A8_UNORM, // BGRA8_UNORM
    VK_FORMAT_B8G8R8A8_SRGB, // BGRA8_SRGB
    VK_FORMAT_R8G8B8A8_UNORM, // RGBA8_UNORM
    VK_FORMAT_R8G8B8A8_SRGB, // RGBA8_SRGB
    VK_FORMAT_R8G8B8A8_SNORM, // RGBA8_SNORM
    VK_FORMAT_R8G8B8A8_UINT, // RGBA8_UINT
    VK_FORMAT_R8G8B8A8_SINT, // RGBA8_SINT
    VK_FORMAT_R16_UNORM, // R16_UNORM
    VK_FORMAT_R16_SNORM, // R16_SNORM
    VK_FORMAT_R16_UINT, // R16_UINT
    VK_FORMAT_R16_SINT, // R16_SINT
    VK_FORMAT_R16_SFLOAT, // R16_SFLOAT
    VK_FORMAT_R16G16_UNORM, // RG16_UNORM
    VK_FORMAT_R16G16_SNORM, // RG16_SNORM
    VK_FORMAT_R16G16_UINT, // RG16_UINT
    VK_FORMAT_R16G16_SINT, // RG16_SINT
    VK_FORMAT_R16G16_SFLOAT, // RG16_SFLOAT
    VK_FORMAT_R16G16B16A16_UNORM, // RGBA16_UNORM
    VK_FORMAT_R16G16B16A16_SNORM, // RGBA16_SNORM
    VK_FORMAT_R16G16B16A16_UINT, // RGBA16_UINT
    VK_FORMAT_R16G16B16A16_SINT, // RGBA16_SINT
    VK_FORMAT_R16G16B16A16_SFLOAT, // RGBA16_SFLOAT
    VK_FORMAT_R32_UINT, // R32_UINT
    VK_FORMAT_R32_SINT, // R32_SINT
    VK_FORMAT_R32_SFLOAT, // R32_SFLOAT
    VK_FORMAT_R32G32_UINT, // RG32_UINT
    VK_FORMAT_R32G32_SINT, // RG32_SINT
    VK_FORMAT_R32G32_SFLOAT, // RG32_SFLOAT
    VK_FORMAT_R32G32B32_UINT, // RGB32_UINT
    VK_FORMAT_R32G32B32_SINT, // RGB32_SINT
    VK_FORMAT_R32G32B32_SFLOAT, // RGB32_SFLOAT
    VK_FORMAT_R32G32B32A32_UINT, // RGB32_UINT
    VK_FORMAT_R32G32B32A32_SINT, // RGB32_SINT
    VK_FORMAT_R32G32B32A32_SFLOAT, // RGB32_SFLOAT
    VK_FORMAT_R5G6B5_UNORM_PACK16, // B5_G6_R5_UNORM
    VK_FORMAT_A1R5G5B5_UNORM_PACK16, // B5_G5_R5_A1_UNORM
    VK_FORMAT_A4R4G4B4_UNORM_PACK16, // B4_G4_R4_A4_UNORM
    VK_FORMAT_A2B10G10R10_UNORM_PACK32, // R10_G10_B10_A2_UNORM
    VK_FORMAT_A2B10G10R10_UINT_PACK32, // R10_G10_B10_A2_UINT
    VK_FORMAT_B10G11R11_UFLOAT_PACK32, // R11_G11_B10_UFLOAT
    VK_FORMAT_E5B9G9R9_UFLOAT_PACK32, // R9_G9_B9_E5_UFLOAT
    VK_FORMAT_BC1_RGBA_UNORM_BLOCK, // BC1_RGBA_UNORM
    VK_FORMAT_BC1_RGBA_SRGB_BLOCK, // BC1_RGBA_SRGB
    VK_FORMAT_BC2_UNORM_BLOCK, // BC2_RGBA_UNORM
    VK_FORMAT_BC2_SRGB_BLOCK, // BC2_RGBA_SRGB
    VK_FORMAT_BC3_UNORM_BLOCK, // BC3_RGBA_UNORM
    VK_FORMAT_BC3_SRGB_BLOCK, // BC3_RGBA_SRGB
    VK_FORMAT_BC4_UNORM_BLOCK, // BC4_R_UNORM
    VK_FORMAT_BC4_SNORM_BLOCK, // BC4_R_SNORM
    VK_FORMAT_BC5_UNORM_BLOCK, // BC5_RG_UNORM
    VK_FORMAT_BC5_SNORM_BLOCK, // BC5_RG_SNORM
    VK_FORMAT_BC6H_UFLOAT_BLOCK, // BC6H_RGB_UFLOAT
    VK_FORMAT_BC6H_SFLOAT_BLOCK, // BC6H_RGB_SFLOAT
    VK_FORMAT_BC7_UNORM_BLOCK, // BC7_RGBA_UNORM
    VK_FORMAT_BC7_SRGB_BLOCK, // BC7_RGBA_SRGB
    VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK, // ETC2_RGB8_UNORM
    VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK, // ETC2_RGB8_SRGB
    VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK, // ETC2_RGB8_A1_UNORM
    VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK, // ETC2_RGB8_A1_SRGB
    VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK, // ETC2_RGB8_A8_UNORM
    VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK, // ETC2_RGB8_A8_SRGB
    VK_FORMAT_EAC_R11_UNORM_BLOCK, // ETC2_R11_UNORM
    VK_FORMAT_EAC_R11_SNORM_BLOCK, // ETC2_R11_SNORM
    VK_FORMAT_EAC_R11G11_UNORM_BLOCK, // ETC2_R11_G11_UNORM
    VK_FORMAT_EAC_R11G11_SNORM_BLOCK, // ETC2_R11_G11_SNORM
    VK_FORMAT_ASTC_4x4_UNORM_BLOCK, // ASTC_4X4_UNORM
    VK_FORMAT_ASTC_4x4_SRGB_BLOCK, // ASTC_4X4_SRGB
    VK_FORMAT_ASTC_5x4_UNORM_BLOCK, // ASTC_5X4_UNORM
    VK_FORMAT_ASTC_5x4_SRGB_BLOCK, // ASTC_5X4_SRGB
    VK_FORMAT_ASTC_5x5_UNORM_BLOCK, // ASTC_5X5_UNORM
    VK_FORMAT_ASTC_5x5_SRGB_BLOCK, // ASTC_5X5_SRGB
    VK_FORMAT_ASTC_6x5_UNORM_BLOCK, // ASTC_6X5_UNORM
    VK_FORMAT_ASTC_6x5_SRGB_BLOCK, // ASTC_6X5_SRGB
    VK_FORMAT_ASTC_6x6_UNORM_BLOCK, // ASTC_6X6_UNORM
    VK_FORMAT_ASTC_6x6_SRGB_BLOCK, // ASTC_6X6_SRGB
    VK_FORMAT_ASTC_8x5_UNORM_BLOCK, // ASTC_8X5_UNORM
    VK_FORMAT_ASTC_8x5_SRGB_BLOCK, // ASTC_8X5_SRGB
    VK_FORMAT_ASTC_8x6_UNORM_BLOCK, // ASTC_8X6_UNORM
    VK_FORMAT_ASTC_8x6_SRGB_BLOCK, // ASTC_8X6_SRGB
    VK_FORMAT_ASTC_8x8_UNORM_BLOCK, // ASTC_8X8_UNORM
    VK_FORMAT_ASTC_8x8_SRGB_BLOCK, // ASTC_8X8_SRGB
    VK_FORMAT_ASTC_10x5_UNORM_BLOCK, // ASTC_10X5_UNORM
    VK_FORMAT_ASTC_10x5_SRGB_BLOCK, // ASTC_10X5_SRGB
    VK_FORMAT_ASTC_10x6_UNORM_BLOCK, // ASTC_10X6_UNORM
    VK_FORMAT_ASTC_10x6_SRGB_BLOCK, // ASTC_10X6_SRGB
    VK_FORMAT_ASTC_10x8_UNORM_BLOCK, // ASTC_10X8_UNORM
    VK_FORMAT_ASTC_10x8_SRGB_BLOCK, // ASTC_10X8_SRGB
    VK_FORMAT_ASTC_10x10_UNORM_BLOCK, // ASTC_10X10_UNORM
    VK_FORMAT_ASTC_10x10_SRGB_BLOCK, // ASTC_10X10_SRGB
    VK_FORMAT_ASTC_12x10_UNORM_BLOCK, // ASTC_12X10_UNORM
    VK_FORMAT_ASTC_12x10_SRGB_BLOCK, // ASTC_12X10_SRGB
    VK_FORMAT_ASTC_12x12_UNORM_BLOCK, // ASTC_12X12_UNORM
    VK_FORMAT_ASTC_12x12_SRGB_BLOCK, // ASTC_12X12_SRGB
    VK_FORMAT_D16_UNORM, // D16_UNORM
    VK_FORMAT_D32_SFLOAT, // D32_SFLOAT
    VK_FORMAT_D24_UNORM_S8_UINT, // D24_UNORM_S8_UINT
    VK_FORMAT_D32_SFLOAT_S8_UINT, // D32_SFLOAT_S8_UINT
  };

  ENGINE_FORCE_INLINE std::string vulkanResultToString(VkResult result) {
    switch (result) {
      case VK_SUCCESS: return "VK_SUCCESS";
      case VK_NOT_READY: return "VK_NOT_READY";
      case VK_TIMEOUT: return "VK_TIMEOUT";
      case VK_EVENT_SET: return "VK_EVENT_SET";
      case VK_EVENT_RESET: return "VK_EVENT_RESET";
      case VK_INCOMPLETE: return "VK_INCOMPLETE";
      case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
      case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
      case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
      case VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
      case VK_ERROR_MEMORY_MAP_FAILED: return "VK_ERROR_MEMORY_MAP_FAILED";
      case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
      case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
      case VK_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT";
      case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
      case VK_ERROR_TOO_MANY_OBJECTS: return "VK_ERROR_TOO_MANY_OBJECTS";
      case VK_ERROR_FORMAT_NOT_SUPPORTED: return "VK_ERROR_FORMAT_NOT_SUPPORTED";
      case VK_ERROR_FRAGMENTED_POOL: return "VK_ERROR_FRAGMENTED_POOL";
      case VK_ERROR_OUT_OF_POOL_MEMORY: return "VK_ERROR_OUT_OF_POOL_MEMORY";
      case VK_ERROR_INVALID_EXTERNAL_HANDLE: return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
      case VK_ERROR_SURFACE_LOST_KHR: return "VK_ERROR_SURFACE_LOST_KHR";
      case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
      case VK_SUBOPTIMAL_KHR: return "VK_SUBOPTIMAL_KHR";
      case VK_ERROR_OUT_OF_DATE_KHR: return "VK_ERROR_OUT_OF_DATE_KHR";
      case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
      case VK_ERROR_VALIDATION_FAILED_EXT: return "VK_ERROR_VALIDATION_FAILED_EXT";
      case VK_ERROR_INVALID_SHADER_NV: return "VK_ERROR_INVALID_SHADER_NV";
      case VK_ERROR_NOT_PERMITTED_EXT: return "VK_ERROR_NOT_PERMITTED_EXT";
      default:
        return "Unknown VkResult: " + std::to_string(result);
    }
  }

  constexpr Result vulkanResultToResult(VkResult vkResult) {
    if (vkResult >= 0)
      return Result::Success;

    switch (vkResult) {
      case VK_ERROR_DEVICE_LOST:
        return Result::DeviceLost;

      case VK_ERROR_SURFACE_LOST_KHR:
      case VK_ERROR_OUT_OF_DATE_KHR:
        return Result::OutOfDate;

      case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
      case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
      case VK_ERROR_FORMAT_NOT_SUPPORTED:
      case VK_ERROR_INCOMPATIBLE_DRIVER:
      case VK_ERROR_FEATURE_NOT_PRESENT:
      case VK_ERROR_EXTENSION_NOT_PRESENT:
      case VK_ERROR_LAYER_NOT_PRESENT:
        return Result::Unsupported;

      case VK_ERROR_OUT_OF_HOST_MEMORY:
      case VK_ERROR_OUT_OF_DEVICE_MEMORY:
      case VK_ERROR_OUT_OF_POOL_MEMORY:
      case VK_ERROR_FRAGMENTATION:
      case VK_ERROR_FRAGMENTED_POOL:
        return Result::OutOfMemory;

      default:
        return Result::Failure;
    }
  }

  inline VkFormat formatToVulkanFormat(Format format, bool demoteSrgb = false) {
    if (demoteSrgb) {
      const FormatProperties &formatProps = getFormatProperties(format);
      if (formatProps.isSrgb)
        format = static_cast<Format>(static_cast<uint32_t>(format) - 1);
    }
    return static_cast<VkFormat>(static_cast<uint32_t>(vulkanFormats[static_cast<uint32_t>(format)]));
  }

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
    VkBool32 memoryZeroInitializationEnabled = VK_FALSE;
  };

  struct VulkanMemoryTypeInfo {
    uint16_t index;
    MemoryLocation location;
    bool mustBeDedicated;
  };

  class VulkanSwapChain final: public SwapChain {
    public:
      VulkanSwapChain(VulkanDevice &device);

      Result create(const SwapChainInfo &swapChainInfo);

      ENGINE_FORCE_INLINE Result acquireNextImage(VulkanFence &acquireSemaphore, uint32_t &textureIndex);

    private:
      VulkanDevice &device;
      Vector<VkImage> textures;
      TextureUsageBits imageUsageFlags = TextureUsageBits::None;

      VulkanFence *latencyFence = nullptr;
      VulkanQueue *presentQueue = nullptr;

      VkSwapchainKHR swapChain = VK_NULL_HANDLE;
      VkSurfaceKHR surface = VK_NULL_HANDLE;

      SDL_Window *windowHandle = nullptr;
      uint64_t presentId = 0;
      uint32_t textureIndex = 0;
      SwapChainBits flags = SwapChainBits::None;
  };

  class VulkanQueue: public Queue {
    public:
      VulkanQueue(VulkanDevice &device);

      ~VulkanQueue() override;

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

  class VulkanDevice final: public Device {
    public:
      VulkanDevice(const CallbackInterface &callbacks, const AllocationCallbacks &allocationCallbacks);

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

      bool extensionSupported(const char *extension, const Vector<VkExtensionProperties> &supportedExtensions) const;

      bool extensionSupported(const char *extension, const Vector<const char*> &supportedExtensions) const;

      Result create(const DeviceCreateInfo &createInfo);

      bool getMemoryInfo(
        MemoryLocation memoryLocation,
        const VkMemoryRequirements &memoryRequirements,
        const VkMemoryDedicatedRequirements &memoryDedicatedRequirements,
        MemoryInfo &memoryInfo
      ) const;

      ENGINE_FORCE_INLINE FormatSupportBits getFormatSupport(Format format) const;

      inline const DeviceInfo &getDeviceInfo() const { return deviceInfo; }

      inline VmaAllocator_T *getVma() const { return vmaAllocator; }

      VkAllocationCallbacks vulkanAllocationCallbacks = {};

      const VulkanDeviceFeatures &getSupportedFeatures() const { return deviceFeatures; }

    private:
      Result createInstance(bool validationLayerEnabled, const Vector<const char*> &enabledExtensions);

      void loadInstanceExtensions(Vector<const char*> &enabledExtensions);

      void loadDeviceExtensions(Vector<const char*> &enabledDeviceExtensions, bool disableRayTracing);

      ENGINE_FORCE_INLINE void addExtension(
        const char *extension,
        Vector<const char*> &enabledExtensions,
        const Vector<VkExtensionProperties> &supportedExtensions
      ) const;

      VkInstance instance = VK_NULL_HANDLE;
      VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
      VkDevice device = VK_NULL_HANDLE;

      VkPhysicalDeviceMemoryProperties memoryProperties = {};

      VulkanDeviceFeatures deviceFeatures = {};
      DeviceInfo deviceInfo = {};
      std::array<Vector<IntrusivePtr<VulkanQueue>>, static_cast<std::size_t>(QueueType::Count)> queueFamilies;

      VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
      VmaAllocator_T *vmaAllocator = nullptr;

      uint8_t majorVersion = 1;
      uint8_t minorVersion = 0;
  };

  class VulkanFence {
    public:
      VulkanFence(VulkanDevice &device);

      ~VulkanFence();

      inline operator VkSemaphore() const {
        return semaphore;
      }

      Result create(uint64_t initialValue);

      void wait(uint64_t value);

    private:
      VulkanDevice &device;
      VkSemaphore semaphore = VK_NULL_HANDLE;
  };

  Result createVulkanDevice(const DeviceCreateInfo &createInfo, Device *&device);
}
