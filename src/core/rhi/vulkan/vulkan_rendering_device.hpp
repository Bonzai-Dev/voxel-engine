#pragma once
#include <core/renderer/renderer.hpp>
#include <core/application/window.hpp>
#include "volk.h"
#include "vulkan_context.hpp"
#include "vulkan_swapchain.hpp"

namespace Core::Graphics {
  enum class PrimitiveTopology {
    None = 0,
    Points,
    Lines,
    Triangles,
    LineStrip,
    TriangleStrip,
    TriangleFan
  };

  inline std::string vulkanResultToString(VkResult result) {
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

  class VulkanRenderingDevice {
    public:
      VulkanRenderingDevice(Backend backend, const char *appName, const DisplayInfo &displayInfo);

      ~VulkanRenderingDevice();

      void aquireNextImage();

      const VulkanContext &getContext() const { return vulkanContext; }

      static constexpr std::uint32_t framesInFlight = 3;

    private:
      Backend backend;
      VulkanContext vulkanContext;
      const char *appName;
      const DisplayInfo &displayInfo;

      VkDescriptorSetLayout descriptorSetLayoutTex;
      VkPipeline pipeline = VK_NULL_HANDLE;
      VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
  };

  #define VULKAN_CHECK(vulkanCall) { \
    VkResult result = vulkanCall; \
    if (result != VK_SUCCESS) { \
      std::string vkfunc = #vulkanCall; \
      vkfunc = vkfunc.substr(0, vkfunc.find('(')); \
      throw std::runtime_error("Vulkan error: " + vkfunc + " failed with " + vulkanResultToString(result)); \
    } \
  }
}
