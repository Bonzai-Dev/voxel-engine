#include <numeric>
#include <SDL3/SDL_vulkan.h>
#include <core/assert.hpp>
#include <core/logger.hpp>
#include "vulkan_backend.hpp"

#define PNEXT_CHAIN_APPEND_FEATURES(extension, structName, supportedExtensions) \
  if (extensionSupported(extension, supportedExtensions)) \
  PNEXT_CHAIN_APPEND_STRUCT(structName)

namespace Core::RHI {
  static inline uint32_t nextPow2(uint32_t n) {
    if (n <= 1)
      return 1;

    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;

    return n;
  }

  static void * VKAPI_PTR vkAllocateHostMemory(void *pUserData, size_t size, size_t alignment, VkSystemAllocationScope) {
    const auto &allocationCallbacks = *(AllocationCallbacks*)pUserData;
    return allocationCallbacks.allocate(allocationCallbacks.userArg, size, alignment);
  }

  static void * VKAPI_PTR vkReallocateHostMemory(
    void *pUserData, void *pOriginal, size_t size, size_t alignment, VkSystemAllocationScope
  ) {
    const auto &allocationCallbacks = *(AllocationCallbacks*)pUserData;
    return allocationCallbacks.reallocate(allocationCallbacks.userArg, pOriginal, size, alignment);
  }

  static void VKAPI_PTR vkFreeHostMemory(void *pUserData, void *pMemory) {
    const auto &allocationCallbacks = *(AllocationCallbacks*)pUserData;
    return allocationCallbacks.free(allocationCallbacks.userArg, pMemory);
  }

  static VkBool32 messageCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *callbackData,
    void *userData
  ) {
    // TODO: some messages can be muted here
    {
      uint32_t messageId = (uint32_t)callbackData->messageIdNumber;
      // Loader info message
      if (messageId == 0)
        return VK_FALSE;
      // Validation Information: [ WARNING-CreateInstance-status-message ] vkCreateInstance(): Khronos Validation Layer Active ...
      if (messageId == 601872502)
        return VK_FALSE;
      // Validation Warning: [ VALIDATION-SETTINGS ] vkCreateInstance(): DebugPrintf logs to the Information message severity, enabling Information level logging otherwise the message will not be seen.
      if (messageId == 2132353751)
        return VK_FALSE;
      // Validation Warning: [ WARNING-DEBUG-PRINTF ] Internal Warning: Setting VkPhysicalDeviceVulkan12Properties::maxUpdateAfterBindDescriptorsInAllPools to 32
      if (messageId == 1985515673)
        return VK_FALSE;
      // vkGetPhysicalDeviceProperties2(): Internal Warning: Setting VkPhysicalDeviceVulkan12Properties::maxUpdateAfterBindDescriptorsInAllPools to 4194304
      if (messageId == 2264819489)
        return VK_FALSE;
    }

    if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
      LOG_CORE_ERROR("{} {} {}", __FILE__, __LINE__, callbackData->pMessage, callbackData->messageIdNumber);
    else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
      LOG_CORE_WARNING("{} {} {}", __FILE__, __LINE__, callbackData->pMessage, callbackData->messageIdNumber);

    return VK_FALSE;
  }

  VulkanDevice::VulkanDevice(const CallbackInterface& callbacks, const AllocationCallbacks& allocationCallbacks):
    Device(callbacks, allocationCallbacks), queueFamilies({
      Vector<IntrusivePtr<VulkanQueue>>(stdAllocator),
      Vector<IntrusivePtr<VulkanQueue>>(stdAllocator),
      Vector<IntrusivePtr<VulkanQueue>>(stdAllocator)}
    )
  {
    vulkanAllocationCallbacks.pUserData = &this->allocationCallbacks;
    vulkanAllocationCallbacks.pfnAllocation = vkAllocateHostMemory;
    vulkanAllocationCallbacks.pfnReallocation = vkReallocateHostMemory;
    vulkanAllocationCallbacks.pfnFree = vkFreeHostMemory;

    deviceInfo.graphicsBackend = GraphicsBackend::Vulkan;
  }

  VulkanDevice::~VulkanDevice() {
    if (vmaAllocator)
      vmaDestroyAllocator(vmaAllocator);

    if (debugMessenger)
      vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, &vulkanAllocationCallbacks);

    if (device)
      vkDestroyDevice(device, &vulkanAllocationCallbacks);

    if (instance)
      vkDestroyInstance(instance, &vulkanAllocationCallbacks);

    volkFinalize();
  }

  Result VulkanDevice::create(const DeviceCreateInfo &createInfo) {
    deviceInfo.physicalDeviceInfo = *createInfo.physicalDeviceInfo;

    // Creating instance
    {
      if (volkInitialize() != VK_SUCCESS) {
        LOG_CORE_CRITICAL("Vulkan is unsupported on the system");
        return Result::Failure;
      }

      Vector<const char*> enabledExtensions(stdAllocator);
      for (uint32_t i = 0; i < createInfo.vulkanExtensions.instanceExtensionCount; i++)
        enabledExtensions.push_back(createInfo.vulkanExtensions.instanceExtensions[i]);

      loadInstanceExtensions(enabledExtensions);

      Result result = createInstance(createInfo.enableGraphicsAPIValidation, enabledExtensions);
      if (result != Result::Success)
        return result;
    }

    // Creating physical device
    {
      uint32_t deviceGroupCount = 0;
      VULKAN_CHECK(vkEnumeratePhysicalDeviceGroups(instance, &deviceGroupCount, nullptr));

      Scratch<VkPhysicalDeviceGroupProperties> deviceGroups = NRI_ALLOCATE_SCRATCH(
        *this, VkPhysicalDeviceGroupProperties, deviceGroupCount
      );
      for (uint32_t i = 0; i < deviceGroupCount; i++) {
        deviceGroups[i].sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES;
        deviceGroups[i].pNext = nullptr;
      }

      VULKAN_CHECK(vkEnumeratePhysicalDeviceGroups(instance, &deviceGroupCount, deviceGroups));

      uint32_t physicalDeviceIndex = 0;
      for (physicalDeviceIndex = 0; physicalDeviceIndex < deviceGroupCount; physicalDeviceIndex++) {
        VkPhysicalDeviceProperties2 deviceProperties = {};
        deviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;

        VkPhysicalDeviceIDProperties deviceIdProperties = {};
        deviceIdProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES;
        deviceProperties.pNext = &deviceIdProperties;

        vkGetPhysicalDeviceProperties2(deviceGroups[physicalDeviceIndex].physicalDevices[0], &deviceProperties);

        majorVersion = VK_API_VERSION_MAJOR(deviceProperties.properties.apiVersion);
        minorVersion = VK_API_VERSION_MINOR(deviceProperties.properties.apiVersion);

        bool deviceSupported = (majorVersion * 10 + minorVersion) >= 12;
        if (deviceSupported) {
          LOG_CORE_INFO("Found {} as a possible rendering device", deviceProperties.properties.deviceName);
          break;
        }
      }

      if (physicalDeviceIndex == deviceGroupCount) {
        LOG_CORE_ERROR("Can't create a device: physical device not found");
        return Result::Unsupported;
      }

      if (
        deviceGroups[physicalDeviceIndex].physicalDeviceCount > 1 &&
        deviceGroups[physicalDeviceIndex].subsetAllocation == VK_FALSE
      )
        LOG_CORE_WARNING("The device group does not support memory allocation on a subset of the physical devices");

      physicalDevice = deviceGroups[physicalDeviceIndex].physicalDevices[0];
      LOG_CORE_INFO("Using {} as the primary rendering device", deviceInfo.physicalDeviceInfo.name);
    }

    // Queue family indices
    std::array<uint32_t, static_cast<size_t>(QueueType::Count)> queueFamilyIndices{};
    queueFamilyIndices.fill(invalidQueueFamilyIndex);
    uint32_t familyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, &familyCount, nullptr);

    Scratch<VkQueueFamilyProperties2> familyProps2 = NRI_ALLOCATE_SCRATCH(*this, VkQueueFamilyProperties2, familyCount);
    for (uint32_t i = 0; i < familyCount; i++)
      familyProps2[i] = {VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2};

    vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, &familyCount, familyProps2);

    std::array<uint32_t, static_cast<size_t>(QueueType::Count)> scores = {};
    for (uint32_t i = 0; i < familyCount; i++) {
      const VkQueueFamilyProperties &familyProps = familyProps2[i].queueFamilyProperties;

      QueueFamilyProperties queueFamilyProperties = {};
      queueFamilyProperties.queueCount = familyProps.queueCount;
      queueFamilyProperties.graphics = familyProps.queueFlags & VK_QUEUE_GRAPHICS_BIT;
      queueFamilyProperties.compute = familyProps.queueFlags & VK_QUEUE_COMPUTE_BIT;
      queueFamilyProperties.copy = familyProps.queueFlags & VK_QUEUE_TRANSFER_BIT;
      queueFamilyProperties.sparse = familyProps.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT;
      queueFamilyProperties.videoDecode = familyProps.queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR;
      queueFamilyProperties.videoEncode = familyProps.queueFlags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR;
      queueFamilyProperties.protect = familyProps.queueFlags & VK_QUEUE_PROTECTED_BIT;
      queueFamilyProperties.opticalFlow = familyProps.queueFlags & VK_QUEUE_OPTICAL_FLOW_BIT_NV;

      QueueType queueType = selectSuitableQueueType(queueFamilyProperties, scores);
      if (queueType != QueueType::Count)
        queueFamilyIndices[static_cast<size_t>(queueType)] = i;
    }

    // Device memory properties
    {
      VkPhysicalDeviceMemoryProperties2 properties = {};
      properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
      vkGetPhysicalDeviceMemoryProperties2(physicalDevice, &properties);
      memoryProperties = properties.memoryProperties;
    }

    // Device extensions
    Vector<const char*> enabledDeviceExtensions(stdAllocator);
    for (uint32_t i = 0; i < createInfo.vulkanExtensions.deviceExtensionCount; i++)
      enabledDeviceExtensions.push_back(createInfo.vulkanExtensions.deviceExtensions[i]);

    loadDeviceExtensions(enabledDeviceExtensions, createInfo.disableVKRayTracing);

    LOG_CORE_INFO(
      "Vulkan {}.{} has been initialized with {} device extensions",
      majorVersion, minorVersion, enabledDeviceExtensions.size()
    );

    // Device features and querying support for device features
    VkPhysicalDeviceFeatures2 deviceFeatures{};
    deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    PNEXT_CHAIN_DECLARE(deviceFeatures.pNext);

    VkPhysicalDeviceVulkan11Features deviceFeatures11{};
    deviceFeatures11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
    PNEXT_CHAIN_APPEND_STRUCT(deviceFeatures11);

    VkPhysicalDeviceVulkan12Features deviceFeatures12{};
    deviceFeatures12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    PNEXT_CHAIN_APPEND_STRUCT(deviceFeatures12);

    VkPhysicalDeviceVulkan13Features deviceFeatures13{};
    deviceFeatures13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    if (minorVersion >= 3)
      PNEXT_CHAIN_APPEND_STRUCT(deviceFeatures13);

    VkPhysicalDeviceVulkan14Features deviceFeatures14{};
    deviceFeatures14.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES;
    if (minorVersion >= 4)
      PNEXT_CHAIN_APPEND_STRUCT(deviceFeatures14);

    /* clang-format off*/
    VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR};
    VkPhysicalDeviceMaintenance4FeaturesKHR maintenance4Features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES_KHR};
    VkPhysicalDeviceSynchronization2FeaturesKHR synchronization2Features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR};
    VkPhysicalDeviceShaderIntegerDotProductFeaturesKHR shaderIntegerDotProductFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_DOT_PRODUCT_FEATURES_KHR};
    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT};
    VkPhysicalDeviceImageRobustnessFeaturesEXT imageRobustnessFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ROBUSTNESS_FEATURES_EXT};

    VkPhysicalDeviceSubgroupSizeControlFeaturesEXT subgroupSizeControlFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_FEATURES_EXT};
    VkPhysicalDevicePipelineCreationCacheControlFeaturesEXT pipelineCreationCacheControlFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_CREATION_CACHE_CONTROL_FEATURES_EXT};

    if (minorVersion < 3) {
      PNEXT_CHAIN_APPEND_FEATURES(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME, dynamicRenderingFeatures, enabledDeviceExtensions);
      PNEXT_CHAIN_APPEND_FEATURES(VK_KHR_MAINTENANCE_4_EXTENSION_NAME, maintenance4Features, enabledDeviceExtensions);
      PNEXT_CHAIN_APPEND_FEATURES(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME, synchronization2Features, enabledDeviceExtensions);
      PNEXT_CHAIN_APPEND_FEATURES(VK_KHR_SHADER_INTEGER_DOT_PRODUCT_EXTENSION_NAME, shaderIntegerDotProductFeatures, enabledDeviceExtensions);
      PNEXT_CHAIN_APPEND_FEATURES(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME, extendedDynamicStateFeatures, enabledDeviceExtensions);
      PNEXT_CHAIN_APPEND_FEATURES(VK_EXT_IMAGE_ROBUSTNESS_EXTENSION_NAME, imageRobustnessFeatures, enabledDeviceExtensions);
      PNEXT_CHAIN_APPEND_FEATURES(VK_EXT_SUBGROUP_SIZE_CONTROL_EXTENSION_NAME, subgroupSizeControlFeatures, enabledDeviceExtensions);
      PNEXT_CHAIN_APPEND_FEATURES(VK_EXT_PIPELINE_CREATION_CACHE_CONTROL_EXTENSION_NAME, pipelineCreationCacheControlFeatures, enabledDeviceExtensions);
    }

    VkPhysicalDeviceLineRasterizationFeaturesKHR lineRasterizationFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES_KHR};
    VkPhysicalDeviceMaintenance5FeaturesKHR maintenance5Features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES_KHR};
    VkPhysicalDeviceMaintenance6FeaturesKHR maintenance6Features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_6_FEATURES_KHR};
    VkPhysicalDevicePipelineRobustnessFeaturesEXT pipelineRobustnessFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_ROBUSTNESS_FEATURES_EXT};

    if (minorVersion < 4) {
      PNEXT_CHAIN_APPEND_FEATURES(VK_KHR_LINE_RASTERIZATION_EXTENSION_NAME, lineRasterizationFeatures, enabledDeviceExtensions);
      PNEXT_CHAIN_APPEND_FEATURES(VK_KHR_MAINTENANCE_5_EXTENSION_NAME, maintenance5Features, enabledDeviceExtensions);
      PNEXT_CHAIN_APPEND_FEATURES(VK_KHR_MAINTENANCE_6_EXTENSION_NAME, maintenance6Features, enabledDeviceExtensions);
      PNEXT_CHAIN_APPEND_FEATURES(VK_EXT_PIPELINE_ROBUSTNESS_EXTENSION_NAME, pipelineRobustnessFeatures, enabledDeviceExtensions);
    }

    VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR};
    VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR computeShaderDerivativesFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMPUTE_SHADER_DERIVATIVES_FEATURES_KHR};
    VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR fragmentShaderBarycentricFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_FEATURES_KHR};
    VkPhysicalDeviceFragmentShadingRateFeaturesKHR fragmentShadingRateFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR};

    VkPhysicalDeviceMaintenance7FeaturesKHR maintenance7Features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_7_FEATURES_KHR};
    VkPhysicalDeviceMaintenance8FeaturesKHR maintenance8Features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_8_FEATURES_KHR};
    VkPhysicalDeviceMaintenance9FeaturesKHR maintenance9Features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_9_FEATURES_KHR};
    VkPhysicalDeviceMaintenance10FeaturesKHR maintenance10Features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_10_FEATURES_KHR};

    VkPhysicalDevicePresentIdFeaturesKHR presentIdFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_ID_FEATURES_KHR};
    VkPhysicalDevicePresentWaitFeaturesKHR presentWaitFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_WAIT_FEATURES_KHR};

    VkPhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR};
    VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR rayTracingMaintenance1Features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_MAINTENANCE_1_FEATURES_KHR};
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR};
    VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR rayTracingPositionFetchFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_POSITION_FETCH_FEATURES_KHR};

    VkPhysicalDeviceShaderClockFeaturesKHR shaderClockFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CLOCK_FEATURES_KHR};
    VkPhysicalDeviceDynamicRenderingLocalReadFeaturesKHR dynamicRenderingLocalReadFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_LOCAL_READ_FEATURES_KHR};
    VkPhysicalDeviceUnifiedImageLayoutsFeaturesKHR unifiedImageLayoutsFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFIED_IMAGE_LAYOUTS_FEATURES_KHR};
    VkPhysicalDeviceCustomBorderColorFeaturesEXT customBorderColorFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_FEATURES_EXT};
    VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT fragmentShaderInterlockFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_INTERLOCK_FEATURES_EXT};
    VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT imageSlicedViewOf3DFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_SLICED_VIEW_OF_3D_FEATURES_EXT};

    VkPhysicalDeviceMemoryPriorityFeaturesEXT memoryPriorityFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PRIORITY_FEATURES_EXT};
    VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT};

    VkPhysicalDeviceOpacityMicromapFeaturesEXT opacityMicromapFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPACITY_MICROMAP_FEATURES_EXT};
    VkPhysicalDevicePresentModeFifoLatestReadyFeaturesEXT presentModeFifoLatestReadyFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_MODE_FIFO_LATEST_READY_FEATURES_EXT};
    VkPhysicalDeviceRobustness2FeaturesEXT robustness2Features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT};

    VkPhysicalDeviceShaderAtomicFloatFeaturesEXT shaderAtomicFloatFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_FEATURES_EXT};
    VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT shaderAtomicFloat2Features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_2_FEATURES_EXT};
    VkPhysicalDeviceSwapchainMaintenance1FeaturesEXT swapChainMaintenance1Features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SWAPCHAIN_MAINTENANCE_1_FEATURES_EXT};
    VkPhysicalDeviceZeroInitializeDeviceMemoryFeaturesEXT zeroInitializeDeviceMemoryFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ZERO_INITIALIZE_DEVICE_MEMORY_FEATURES_EXT};
    VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT mutableDescriptorTypeFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MUTABLE_DESCRIPTOR_TYPE_FEATURES_EXT};

    PNEXT_CHAIN_APPEND_FEATURES(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, accelerationStructureFeatures, enabledDeviceExtensions);
    PNEXT_CHAIN_APPEND_FEATURES(VK_KHR_COMPUTE_SHADER_DERIVATIVES_EXTENSION_NAME, computeShaderDerivativesFeatures, enabledDeviceExtensions);
    PNEXT_CHAIN_APPEND_FEATURES(VK_KHR_FRAGMENT_SHADER_BARYCENTRIC_EXTENSION_NAME, fragmentShaderBarycentricFeatures, enabledDeviceExtensions);
    PNEXT_CHAIN_APPEND_FEATURES(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME, fragmentShadingRateFeatures, enabledDeviceExtensions);
    PNEXT_CHAIN_APPEND_FEATURES(VK_KHR_MAINTENANCE_7_EXTENSION_NAME, maintenance7Features, enabledDeviceExtensions);
    PNEXT_CHAIN_APPEND_FEATURES(VK_KHR_MAINTENANCE_8_EXTENSION_NAME, maintenance8Features, enabledDeviceExtensions);
    PNEXT_CHAIN_APPEND_FEATURES(VK_KHR_MAINTENANCE_9_EXTENSION_NAME, maintenance9Features, enabledDeviceExtensions);
    PNEXT_CHAIN_APPEND_FEATURES(VK_KHR_MAINTENANCE_10_EXTENSION_NAME, maintenance10Features, enabledDeviceExtensions);
    PNEXT_CHAIN_APPEND_FEATURES(VK_KHR_PRESENT_ID_EXTENSION_NAME, presentIdFeatures, enabledDeviceExtensions);
    PNEXT_CHAIN_APPEND_FEATURES(VK_KHR_PRESENT_WAIT_EXTENSION_NAME, presentWaitFeatures, enabledDeviceExtensions);
    PNEXT_CHAIN_APPEND_FEATURES(VK_KHR_RAY_QUERY_EXTENSION_NAME, rayQueryFeatures, enabledDeviceExtensions);
    PNEXT_CHAIN_APPEND_FEATURES(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME, rayTracingMaintenance1Features, enabledDeviceExtensions);
    PNEXT_CHAIN_APPEND_FEATURES(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, rayTracingPipelineFeatures, enabledDeviceExtensions);
    PNEXT_CHAIN_APPEND_FEATURES(VK_KHR_RAY_TRACING_POSITION_FETCH_EXTENSION_NAME, rayTracingPositionFetchFeatures, enabledDeviceExtensions);
    PNEXT_CHAIN_APPEND_FEATURES(VK_KHR_SHADER_CLOCK_EXTENSION_NAME, shaderClockFeatures, enabledDeviceExtensions);
    PNEXT_CHAIN_APPEND_FEATURES(VK_KHR_DYNAMIC_RENDERING_LOCAL_READ_EXTENSION_NAME, dynamicRenderingLocalReadFeatures, enabledDeviceExtensions);
    PNEXT_CHAIN_APPEND_FEATURES(VK_KHR_UNIFIED_IMAGE_LAYOUTS_EXTENSION_NAME, unifiedImageLayoutsFeatures, enabledDeviceExtensions);
    PNEXT_CHAIN_APPEND_FEATURES(VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME, customBorderColorFeatures, enabledDeviceExtensions);
    PNEXT_CHAIN_APPEND_FEATURES(VK_EXT_FRAGMENT_SHADER_INTERLOCK_EXTENSION_NAME, fragmentShaderInterlockFeatures, enabledDeviceExtensions);
    PNEXT_CHAIN_APPEND_FEATURES(VK_EXT_IMAGE_SLICED_VIEW_OF_3D_EXTENSION_NAME, imageSlicedViewOf3DFeatures, enabledDeviceExtensions);
    PNEXT_CHAIN_APPEND_FEATURES(VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME, memoryPriorityFeatures, enabledDeviceExtensions);
    PNEXT_CHAIN_APPEND_FEATURES(VK_EXT_MESH_SHADER_EXTENSION_NAME, meshShaderFeatures, enabledDeviceExtensions);
    PNEXT_CHAIN_APPEND_FEATURES(VK_EXT_OPACITY_MICROMAP_EXTENSION_NAME, opacityMicromapFeatures, enabledDeviceExtensions);
    PNEXT_CHAIN_APPEND_FEATURES(VK_EXT_PRESENT_MODE_FIFO_LATEST_READY_EXTENSION_NAME, presentModeFifoLatestReadyFeatures, enabledDeviceExtensions);
    PNEXT_CHAIN_APPEND_FEATURES(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME, robustness2Features, enabledDeviceExtensions);
    PNEXT_CHAIN_APPEND_FEATURES(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME, shaderAtomicFloatFeatures, enabledDeviceExtensions);
    PNEXT_CHAIN_APPEND_FEATURES(VK_EXT_SHADER_ATOMIC_FLOAT_2_EXTENSION_NAME, shaderAtomicFloat2Features, enabledDeviceExtensions);
    PNEXT_CHAIN_APPEND_FEATURES(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME, swapChainMaintenance1Features, enabledDeviceExtensions);
    PNEXT_CHAIN_APPEND_FEATURES(VK_EXT_ZERO_INITIALIZE_DEVICE_MEMORY_EXTENSION_NAME, zeroInitializeDeviceMemoryFeatures, enabledDeviceExtensions);
    PNEXT_CHAIN_APPEND_FEATURES(VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME, mutableDescriptorTypeFeatures, enabledDeviceExtensions);
    /* clang-format on*/

    vkGetPhysicalDeviceFeatures2(physicalDevice, &deviceFeatures);

    if (minorVersion < 3) {
      deviceFeatures13.dynamicRendering = dynamicRenderingFeatures.dynamicRendering;
      deviceFeatures13.synchronization2 = synchronization2Features.synchronization2;
      deviceFeatures13.maintenance4 = maintenance4Features.maintenance4;
      deviceFeatures13.robustImageAccess = imageRobustnessFeatures.robustImageAccess;
      deviceFeatures13.shaderIntegerDotProduct = shaderIntegerDotProductFeatures.shaderIntegerDotProduct;
      deviceFeatures13.pipelineCreationCacheControl = pipelineCreationCacheControlFeatures.pipelineCreationCacheControl;
    }

    if (minorVersion < 4) {
      deviceFeatures14.maintenance5 = maintenance5Features.maintenance5;
      deviceFeatures14.maintenance6 = maintenance6Features.maintenance6;
      deviceFeatures14.pipelineRobustness = pipelineRobustnessFeatures.pipelineRobustness;
      deviceFeatures14.rectangularLines = lineRasterizationFeatures.rectangularLines;
      deviceFeatures14.bresenhamLines = lineRasterizationFeatures.bresenhamLines;
      deviceFeatures14.smoothLines = lineRasterizationFeatures.smoothLines;
      deviceFeatures14.stippledRectangularLines = lineRasterizationFeatures.stippledRectangularLines;
      deviceFeatures14.stippledBresenhamLines = lineRasterizationFeatures.stippledBresenhamLines;
      deviceFeatures14.stippledSmoothLines = lineRasterizationFeatures.stippledSmoothLines;
      deviceFeatures14.dynamicRenderingLocalRead = dynamicRenderingLocalReadFeatures.dynamicRenderingLocalRead;
    }

    if (minorVersion > 2)
      extendedDynamicStateFeatures.extendedDynamicState = true;

    this->deviceFeatures.maintenance4 = deviceFeatures13.maintenance4;
    this->deviceFeatures.maintenance5 = deviceFeatures14.maintenance5;
    this->deviceFeatures.maintenance6 = deviceFeatures14.maintenance6;
    this->deviceFeatures.maintenance7 = maintenance7Features.maintenance7;
    this->deviceFeatures.maintenance8 = maintenance8Features.maintenance8;
    this->deviceFeatures.maintenance9 = maintenance9Features.maintenance9;
    this->deviceFeatures.maintenance10 = maintenance10Features.maintenance10;
    this->deviceFeatures.deviceAddress = deviceFeatures12.bufferDeviceAddress;
    this->deviceFeatures.dynamicRendering = deviceFeatures13.dynamicRendering;
    this->deviceFeatures.copyCommands2 = minorVersion > 2 ||
      extensionSupported(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME, enabledDeviceExtensions);
    this->deviceFeatures.swapChainMutableFormat =
      extensionSupported(VK_KHR_SWAPCHAIN_MUTABLE_FORMAT_EXTENSION_NAME, enabledDeviceExtensions);
    this->deviceFeatures.presentId = presentIdFeatures.presentId;
    this->deviceFeatures.memoryPriority = memoryPriorityFeatures.memoryPriority;
    this->deviceFeatures.memoryBudget =
      extensionSupported(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME, enabledDeviceExtensions);
    this->deviceFeatures.imageSlicedView = imageSlicedViewOf3DFeatures.imageSlicedViewOf3D != 0;
    this->deviceFeatures.customBorderColor = customBorderColorFeatures.customBorderColors != 0 &&
      customBorderColorFeatures.customBorderColorWithoutFormat != 0;
    this->deviceFeatures.robustness = deviceFeatures.features.robustBufferAccess != 0 &&
      deviceFeatures13.robustImageAccess != 0;
    this->deviceFeatures.robustness2 = robustness2Features.robustBufferAccess2 != 0 &&
      robustness2Features.robustImageAccess2 != 0;
    this->deviceFeatures.pipelineRobustness = deviceFeatures14.pipelineRobustness;
    this->deviceFeatures.swapChainMaintenance1 = swapChainMaintenance1Features.swapchainMaintenance1;
    this->deviceFeatures.fifoLatestReady = presentModeFifoLatestReadyFeatures.presentModeFifoLatestReady;
    this->deviceFeatures.unifiedImageLayoutsVideo = unifiedImageLayoutsFeatures.unifiedImageLayoutsVideo;

    this->deviceFeatures.memoryZeroInitializationEnabled = createInfo.enableMemoryZeroInitialization &&
      zeroInitializeDeviceMemoryFeatures.zeroInitializeDeviceMemory;

    // Check hard requirements
    if (!deviceFeatures13.synchronization2) {
      LOG_CORE_ERROR("{} is unsupported for the current device", VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
      return Result::Unsupported;
    }

    // Check soft requirement
    if (!extendedDynamicStateFeatures.extendedDynamicState)
      LOG_CORE_DEBUG(
      "{} is unsupported for the current device, using static vertex input stride and fixed viewport/scissor counts",
      VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME
    );

    if (!deviceFeatures13.dynamicRendering)
      LOG_CORE_DEBUG(
      "{} is unsupported for the current device, using render passes instead",
      VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME
    );

    // Creating device
    {
      // Disable undesired features
      if (createInfo.robustness == Robustness::Default || createInfo.robustness == Robustness::Vulkan) {
        robustness2Features.robustBufferAccess2 = 0;
        robustness2Features.robustImageAccess2 = 0;
      }
      else if (createInfo.robustness == Robustness::Off) {
        robustness2Features.robustBufferAccess2 = 0;
        robustness2Features.robustImageAccess2 = 0;
        deviceFeatures.features.robustBufferAccess = 0;
        deviceFeatures13.robustImageAccess = 0;
      }

      std::array<VkDeviceQueueCreateInfo, static_cast<size_t>(QueueType::Count)> queueCreateInfos = {};

      VkDeviceCreateInfo deviceCreateInfo = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
      deviceCreateInfo.pNext = &deviceFeatures;
      deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
      deviceCreateInfo.enabledExtensionCount = (uint32_t)enabledDeviceExtensions.size();
      deviceCreateInfo.ppEnabledExtensionNames = enabledDeviceExtensions.data();

      std::array<float, 256> zeroPriorities = {};
      for (uint32_t i = 0; i < createInfo.queueFamilyCount; i++) {
        const QueueFamilyInfo &queueFamily = createInfo.queueFamilies[i];
        uint32_t queueFamilyIndex = queueFamilyIndices[(size_t)queueFamily.queueType];

        if (queueFamily.queueCount && queueFamilyIndex != invalidQueueFamilyIndex) {
          VkDeviceQueueCreateInfo &queueCreateInfo = queueCreateInfos[deviceCreateInfo.queueCreateInfoCount++];

          queueCreateInfo = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
          queueCreateInfo.queueCount = queueFamily.queueCount;
          queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
          queueCreateInfo.pQueuePriorities = queueFamily.queuePriorities
                                               ? queueFamily.queuePriorities
                                               : zeroPriorities.data();
        }
      }

      VULKAN_CHECK(vkCreateDevice(physicalDevice, &deviceCreateInfo, &vulkanAllocationCallbacks, &device));
      volkLoadDevice(device);
    }

    // Creating queues
    memset(deviceInfo.physicalDeviceInfo.queueCount.data(), 0, sizeof(deviceInfo.physicalDeviceInfo.queueCount));
    // patch to reflect available queues
    {
      for (uint32_t i = 0; i < createInfo.queueFamilyCount; i++) {
        const QueueFamilyInfo &queueFamilyDesc = createInfo.queueFamilies[i];
        auto &queueFamily = queueFamilies[static_cast<size_t>(queueFamilyDesc.queueType)];
        uint32_t queueFamilyIndex = queueFamilyIndices[static_cast<size_t>(queueFamilyDesc.queueType)];

        if (queueFamilyIndex != invalidQueueFamilyIndex) {
          for (uint32_t j = 0; j < queueFamilyDesc.queueCount; j++) {
            VkDeviceQueueInfo2 queueInfo = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2};
            queueInfo.queueFamilyIndex = queueFamilyIndices[static_cast<size_t>(queueFamilyDesc.queueType)];
            queueInfo.queueIndex = j;

            VkQueue handle = VK_NULL_HANDLE;
            vkGetDeviceQueue2(device, &queueInfo, &handle);

            IntrusivePtr<VulkanQueue> queue = IntrusivePtr<VulkanQueue>::create(*this);
            if (queue->create(queueFamilyDesc.queueType, queueInfo.queueFamilyIndex, handle) == Result::Success)
              queueFamily.push_back(queue);
          }

          deviceInfo.physicalDeviceInfo.queueCount[(size_t)queueFamilyDesc.queueType] = queueFamilyDesc.queueCount;
        }
        else
          deviceInfo.physicalDeviceInfo.queueCount[(size_t)queueFamilyDesc.queueType] = 0;
      }
    }

    // Device properties
    {
      VkPhysicalDeviceProperties2 deviceProperties = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
      PNEXT_CHAIN_SET(deviceProperties.pNext);

      VkPhysicalDeviceMaintenance3Properties extraProperties = {
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_3_PROPERTIES
      };
      PNEXT_CHAIN_APPEND_STRUCT(extraProperties);

      VkPhysicalDeviceVulkan11Properties deviceProperties11 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES};
      PNEXT_CHAIN_APPEND_STRUCT(deviceProperties11);

      VkPhysicalDeviceVulkan12Properties deviceProperties12 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES};
      PNEXT_CHAIN_APPEND_STRUCT(deviceProperties12);

      VkPhysicalDeviceVulkan13Properties deviceProperties13 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES};
      if (minorVersion >= 3)
        PNEXT_CHAIN_APPEND_STRUCT(deviceProperties13);

      VkPhysicalDeviceVulkan14Properties deviceProperties14 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_PROPERTIES};
      if (minorVersion >= 4)
        PNEXT_CHAIN_APPEND_STRUCT(deviceProperties14);

      VkPhysicalDevicePipelineRobustnessProperties propsRobustness = {
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_ROBUSTNESS_PROPERTIES
      };
      if (minorVersion >= 4)
        PNEXT_CHAIN_APPEND_STRUCT(propsRobustness);

      /* clang-format off*/
      VkPhysicalDeviceMaintenance4PropertiesKHR maintenance4Props = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_PROPERTIES_KHR};
      VkPhysicalDeviceSubgroupSizeControlPropertiesEXT subgroupSizeControlProps = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_PROPERTIES_EXT};

      VkPhysicalDevicePushDescriptorPropertiesKHR pushDescriptorProps = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PUSH_DESCRIPTOR_PROPERTIES_KHR};
      VkPhysicalDeviceMaintenance5PropertiesKHR maintenance5Props = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_PROPERTIES_KHR};
      VkPhysicalDeviceMaintenance6PropertiesKHR maintenance6Props = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_6_PROPERTIES_KHR};

      VkPhysicalDeviceAccelerationStructurePropertiesKHR accelerationStructureProps = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR};
      VkPhysicalDeviceComputeShaderDerivativesPropertiesKHR computeShaderDerivativesProps = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMPUTE_SHADER_DERIVATIVES_PROPERTIES_KHR};
      VkPhysicalDeviceFragmentShadingRatePropertiesKHR fragmentShadingRateProps = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_PROPERTIES_KHR};
      VkPhysicalDeviceLineRasterizationPropertiesKHR lineRasterizationProps = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_PROPERTIES_KHR};
      VkPhysicalDeviceMaintenance7PropertiesKHR maintenance7Props = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_7_PROPERTIES_KHR};
      VkPhysicalDeviceMaintenance9PropertiesKHR maintenance9Props = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_9_PROPERTIES_KHR};
      VkPhysicalDeviceMaintenance10PropertiesKHR maintenance10Props = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_10_PROPERTIES_KHR};
      VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProps = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR};
      VkPhysicalDeviceConservativeRasterizationPropertiesEXT conservativeRasterizationProps = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONSERVATIVE_RASTERIZATION_PROPERTIES_EXT};
      VkPhysicalDeviceMeshShaderPropertiesEXT meshShaderProps = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT};
      VkPhysicalDeviceOpacityMicromapPropertiesEXT opacityMicromapProps = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPACITY_MICROMAP_PROPERTIES_EXT};
      VkPhysicalDeviceSampleLocationsPropertiesEXT sampleLocationsProps = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLE_LOCATIONS_PROPERTIES_EXT};

      if (minorVersion < 3) {
        PNEXT_CHAIN_APPEND_STRUCT(maintenance4Props);
        PNEXT_CHAIN_APPEND_STRUCT(subgroupSizeControlProps);
      }

      if (minorVersion < 4) {
        PNEXT_CHAIN_APPEND_STRUCT(pushDescriptorProps);
        PNEXT_CHAIN_APPEND_STRUCT(maintenance5Props);
        PNEXT_CHAIN_APPEND_STRUCT(maintenance6Props);
      }

      PNEXT_CHAIN_APPEND_STRUCT(accelerationStructureProps);
      PNEXT_CHAIN_APPEND_STRUCT(computeShaderDerivativesProps);
      PNEXT_CHAIN_APPEND_STRUCT(fragmentShadingRateProps);
      PNEXT_CHAIN_APPEND_STRUCT(lineRasterizationProps);
      PNEXT_CHAIN_APPEND_STRUCT(maintenance7Props);
      PNEXT_CHAIN_APPEND_STRUCT(maintenance9Props);
      PNEXT_CHAIN_APPEND_STRUCT(maintenance10Props);
      PNEXT_CHAIN_APPEND_STRUCT(rayTracingPipelineProps);
      PNEXT_CHAIN_APPEND_STRUCT(conservativeRasterizationProps);
      PNEXT_CHAIN_APPEND_STRUCT(meshShaderProps);
      PNEXT_CHAIN_APPEND_STRUCT(opacityMicromapProps);
      PNEXT_CHAIN_APPEND_STRUCT(sampleLocationsProps);

      vkGetPhysicalDeviceProperties2(physicalDevice, &deviceProperties);
      /* clang-format on*/

      if (minorVersion < 3) {
        deviceProperties13.maxBufferSize = maintenance4Props.maxBufferSize;
        deviceProperties13.minSubgroupSize = subgroupSizeControlProps.minSubgroupSize;
        deviceProperties13.maxSubgroupSize = subgroupSizeControlProps.minSubgroupSize;
      }

      if (minorVersion < 4) {
        deviceProperties14.earlyFragmentMultisampleCoverageAfterSampleCounting = maintenance5Props.
          earlyFragmentMultisampleCoverageAfterSampleCounting;
        deviceProperties14.earlyFragmentSampleMaskTestBeforeSampleCounting = maintenance5Props.
          earlyFragmentSampleMaskTestBeforeSampleCounting;
        deviceProperties14.depthStencilSwizzleOneSupport = maintenance5Props.depthStencilSwizzleOneSupport;
        deviceProperties14.polygonModePointSize = maintenance5Props.polygonModePointSize;
        deviceProperties14.nonStrictSinglePixelWideLinesUseParallelogram = maintenance5Props.
          nonStrictSinglePixelWideLinesUseParallelogram;
        deviceProperties14.nonStrictWideLinesUseParallelogram = maintenance5Props.nonStrictWideLinesUseParallelogram;

        deviceProperties14.blockTexelViewCompatibleMultipleLayers = maintenance6Props.
          blockTexelViewCompatibleMultipleLayers;
        deviceProperties14.maxCombinedImageSamplerDescriptorCount = maintenance6Props.
          maxCombinedImageSamplerDescriptorCount;
        deviceProperties14.fragmentShadingRateClampCombinerInputs = maintenance6Props.
          fragmentShadingRateClampCombinerInputs;

        deviceProperties14.maxPushDescriptors = pushDescriptorProps.maxPushDescriptors;

        // These "local read" features were "all or nothing" before 1.4
        deviceProperties14.dynamicRenderingLocalReadDepthStencilAttachments = dynamicRenderingLocalReadFeatures.
          dynamicRenderingLocalRead;
        deviceProperties14.dynamicRenderingLocalReadMultisampledAttachments = dynamicRenderingLocalReadFeatures.
          dynamicRenderingLocalRead;
      }

      // Fill desc
      const VkPhysicalDeviceLimits &limits = deviceProperties.properties.limits;

      uint32_t queueFamilyCount = 0;
      vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, &queueFamilyCount, nullptr);

      Scratch<VkQueueFamilyProperties2> queueFamilyProps2 = NRI_ALLOCATE_SCRATCH(
        *this, VkQueueFamilyProperties2, queueFamilyCount);
      for (uint32_t i = 0; i < queueFamilyCount; i++)
        queueFamilyProps2[i] = {VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2};

      vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, &queueFamilyCount, queueFamilyProps2);

      std::array<bool, static_cast<size_t>(QueueType::Count)> isTimestampSupported = {};
      for (size_t i = 0; i < isTimestampSupported.size(); i++) {
        uint32_t familyIndex = queueFamilyIndices[i];
        isTimestampSupported[i] = deviceInfo.physicalDeviceInfo.queueCount[i] != 0 && familyIndex !=
          invalidQueueFamilyIndex &&
          familyIndex < queueFamilyCount && queueFamilyProps2[familyIndex].queueFamilyProperties.timestampValidBits !=
          0;
      }

      deviceInfo.viewport.maxNum = limits.maxViewports;
      deviceInfo.viewport.boundsMin = (int32_t)limits.viewportBoundsRange[0];
      deviceInfo.viewport.boundsMax = (int32_t)limits.viewportBoundsRange[1];

      deviceInfo.dimensions.attachmentMaxDim = (uint16_t)std::min(limits.maxFramebufferWidth,
                                                                  limits.maxFramebufferHeight);
      deviceInfo.dimensions.attachmentLayerMaxNum = (uint16_t)limits.maxFramebufferLayers;
      deviceInfo.dimensions.texture1DMaxDim = (uint16_t)limits.maxImageDimension1D;
      deviceInfo.dimensions.texture2DMaxDim = (uint16_t)limits.maxImageDimension2D;
      deviceInfo.dimensions.texture3DMaxDim = (uint16_t)limits.maxImageDimension3D;
      deviceInfo.dimensions.textureLayerMaxNum = (uint16_t)limits.maxImageArrayLayers;
      deviceInfo.dimensions.typedBufferMaxDim = limits.maxTexelBufferElements;

      deviceInfo.precision.viewportBits = limits.viewportSubPixelBits;
      deviceInfo.precision.subPixelBits = limits.subPixelPrecisionBits;
      deviceInfo.precision.subTexelBits = limits.subTexelPrecisionBits;
      deviceInfo.precision.mipmapBits = limits.mipmapPrecisionBits;

      const VkMemoryPropertyFlags neededFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
      for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
        const VkMemoryType &memoryType = memoryProperties.memoryTypes[i];
        if ((memoryType.propertyFlags & neededFlags) == neededFlags)
          deviceInfo.memory.deviceUploadHeapSize += memoryProperties.memoryHeaps[memoryType.heapIndex].size;
      }

      /* clang-format off */
      deviceInfo.memory.allocationMaxSize = extraProperties.maxMemoryAllocationSize;
      deviceInfo.memory.allocationMaxNum = limits.maxMemoryAllocationCount;
      deviceInfo.memory.samplerAllocationMaxNum = limits.maxSamplerAllocationCount;
      deviceInfo.memory.constantBufferMaxRange = limits.maxUniformBufferRange;
      deviceInfo.memory.storageBufferMaxRange = limits.maxStorageBufferRange;
      deviceInfo.memory.bufferTextureGranularity = nextPow2((uint32_t)limits.bufferImageGranularity); // there is a vendor returning "9990"!
      deviceInfo.memory.bufferMaxSize = deviceProperties13.maxBufferSize;

      // VUID-VkCopyBufferToImageInfo2-dstImage-07975: If "dstImage" does not have either a depth/stencil format or a multi-planar format,
      //      "bufferOffset" must be a multiple of the texel block size
      // VUID-VkCopyBufferToImageInfo2-dstImage-07978: If "dstImage" has a depth/stencil format,
      //      "bufferOffset" must be a multiple of 4
      // Least Common Multiple stride across all formats: 1, 2, 4, 8, 16 // TODO: rarely used "12" fucks up the beauty of power-of-2 numbers, such formats must be avoided!
      constexpr uint32_t leastCommonMultipleStrideAccrossAllFormats = 16;

      deviceInfo.memoryAlignment.uploadBufferTextureRow = (uint32_t)limits.optimalBufferCopyRowPitchAlignment;
      deviceInfo.memoryAlignment.uploadBufferTextureSlice = std::lcm((uint32_t)limits.optimalBufferCopyOffsetAlignment,
                                                                 leastCommonMultipleStrideAccrossAllFormats);
      deviceInfo.memoryAlignment.bufferShaderResourceOffset = std::lcm((uint32_t)limits.minTexelBufferOffsetAlignment,
                                                                   (uint32_t)limits.minStorageBufferOffsetAlignment);
      deviceInfo.memoryAlignment.constantBufferOffset = (uint32_t)limits.minUniformBufferOffsetAlignment;
      deviceInfo.memoryAlignment.scratchBufferOffset = accelerationStructureProps.minAccelerationStructureScratchOffsetAlignment;
      deviceInfo.memoryAlignment.shaderBindingTable = rayTracingPipelineProps.shaderGroupBaseAlignment;
      deviceInfo.memoryAlignment.accelerationStructureOffset = 256; // see the spec
      deviceInfo.memoryAlignment.micromapOffset = 256; // see the spec
      /* clang-format on */

      {
        // Estimate "worst-case" alignment
        // 1. Buffers
        uint32_t alignment = deviceInfo.memoryAlignment.bufferShaderResourceOffset;
        alignment = std::max(alignment, deviceInfo.memoryAlignment.constantBufferOffset);
        alignment = std::max(alignment, deviceInfo.memoryAlignment.scratchBufferOffset);
        alignment = std::max(alignment, deviceInfo.memoryAlignment.shaderBindingTable);
        alignment = std::max(alignment, deviceInfo.memoryAlignment.accelerationStructureOffset);
        alignment = std::max(alignment, deviceInfo.memoryAlignment.micromapOffset);

        // 2. Buffer-texture neighborhood requirement
        alignment = std::max(alignment, deviceInfo.memory.bufferTextureGranularity);

        // 3. Large non-MSAA texture
        MemoryInfo memoryInfo = {};

        TextureInfo textureInfo = {};
        textureInfo.dimension = TextureDimension::Dimension2D;
        textureInfo.usage = TextureUsageBits::ColorAttachment;
        textureInfo.format = Format::RGBA8_UNORM;
        textureInfo.width = 4096;
        textureInfo.height = 4096;

        IntrusivePtr<VulkanTexture> texture = IntrusivePtr<VulkanTexture>::create(*this);
        texture->create(textureInfo);
        texture->getMemoryInfo(MemoryLocation::Device, memoryInfo);
        uint32_t textureAlignment = memoryInfo.alignment;

        // 4. Large MSAA texture
        FormatSupportBits formatSupportBits = getFormatSupport(textureInfo.format);
        if (formatSupportBits & FormatSupportBits::MultiSample4X)
          textureInfo.sampleNum = 4;
        else if (formatSupportBits & FormatSupportBits::Multisample2X)
          textureInfo.sampleNum = 2;

        IntrusivePtr<VulkanTexture> textureMs = IntrusivePtr<VulkanTexture>::create(*this);
        textureMs->create(textureInfo);
        textureMs->getMemoryInfo(MemoryLocation::Device, memoryInfo);
        uint32_t multisampleTextureAlignment = memoryInfo.alignment;

        // Final
        deviceInfo.memory.alignmentDefault = std::max(textureAlignment, alignment);
        deviceInfo.memory.alignmentMultisample = std::max(multisampleTextureAlignment, alignment);
      }

      deviceInfo.pipelineLayout.descriptorSetMaxNum = limits.maxBoundDescriptorSets;
      deviceInfo.pipelineLayout.rootConstantMaxSize = limits.maxPushConstantsSize;
      deviceInfo.pipelineLayout.rootDescriptorMaxNum = deviceProperties14.maxPushDescriptors;

      deviceInfo.descriptorSet.samplerMaxNum = limits.maxDescriptorSetSamplers;
      deviceInfo.descriptorSet.constantBufferMaxNum = limits.maxDescriptorSetUniformBuffers;
      deviceInfo.descriptorSet.storageBufferMaxNum = limits.maxDescriptorSetStorageBuffers;
      deviceInfo.descriptorSet.textureMaxNum = limits.maxDescriptorSetSampledImages;
      deviceInfo.descriptorSet.storageTextureMaxNum = limits.maxDescriptorSetStorageImages;

      deviceInfo.descriptorSet.updateAfterSet.samplerMaxNum = deviceProperties12.
        maxDescriptorSetUpdateAfterBindSamplers;
      deviceInfo.descriptorSet.updateAfterSet.constantBufferMaxNum = deviceProperties12.
        maxDescriptorSetUpdateAfterBindUniformBuffers;
      deviceInfo.descriptorSet.updateAfterSet.storageBufferMaxNum = deviceProperties12.
        maxDescriptorSetUpdateAfterBindStorageBuffers;
      deviceInfo.descriptorSet.updateAfterSet.textureMaxNum = deviceProperties12.
        maxDescriptorSetUpdateAfterBindSampledImages;
      deviceInfo.descriptorSet.updateAfterSet.storageTextureMaxNum = deviceProperties12.
        maxDescriptorSetUpdateAfterBindStorageImages;

      deviceInfo.shaderStage.descriptorSamplerMaxNum = limits.maxPerStageDescriptorSamplers;
      deviceInfo.shaderStage.descriptorConstantBufferMaxNum = limits.maxPerStageDescriptorUniformBuffers;
      deviceInfo.shaderStage.descriptorStorageBufferMaxNum = limits.maxPerStageDescriptorStorageBuffers;
      deviceInfo.shaderStage.descriptorTextureMaxNum = limits.maxPerStageDescriptorSampledImages;
      deviceInfo.shaderStage.descriptorStorageTextureMaxNum = limits.maxPerStageDescriptorStorageImages;
      deviceInfo.shaderStage.resourceMaxNum = limits.maxPerStageResources;

      deviceInfo.shaderStage.updateAfterSet.descriptorSamplerMaxNum = deviceProperties12.
        maxPerStageDescriptorUpdateAfterBindSamplers;
      deviceInfo.shaderStage.updateAfterSet.descriptorConstantBufferMaxNum = deviceProperties12.
        maxPerStageDescriptorUpdateAfterBindUniformBuffers;
      deviceInfo.shaderStage.updateAfterSet.descriptorStorageBufferMaxNum = deviceProperties12.
        maxPerStageDescriptorUpdateAfterBindStorageBuffers;
      deviceInfo.shaderStage.updateAfterSet.descriptorTextureMaxNum = deviceProperties12.
        maxPerStageDescriptorUpdateAfterBindSampledImages;
      deviceInfo.shaderStage.updateAfterSet.descriptorStorageTextureMaxNum = deviceProperties12.
        maxPerStageDescriptorUpdateAfterBindStorageImages;
      deviceInfo.shaderStage.updateAfterSet.resourceMaxNum = deviceProperties12.maxPerStageUpdateAfterBindResources;

      deviceInfo.shaderStage.vertex.attributeMaxNum = limits.maxVertexInputAttributes;
      deviceInfo.shaderStage.vertex.streamMaxNum = limits.maxVertexInputBindings;
      deviceInfo.shaderStage.vertex.outputComponentMaxNum = limits.maxVertexOutputComponents;

      deviceInfo.shaderStage.tesselationControl.generationMaxLevel = (float)limits.maxTessellationGenerationLevel;
      deviceInfo.shaderStage.tesselationControl.patchPointMaxNum = limits.maxTessellationPatchSize;
      deviceInfo.shaderStage.tesselationControl.perVertexInputComponentMaxNum = limits.
        maxTessellationControlPerVertexInputComponents;
      deviceInfo.shaderStage.tesselationControl.perVertexOutputComponentMaxNum = limits.
        maxTessellationControlPerVertexOutputComponents;
      deviceInfo.shaderStage.tesselationControl.perPatchOutputComponentMaxNum = limits.
        maxTessellationControlPerPatchOutputComponents;
      deviceInfo.shaderStage.tesselationControl.totalOutputComponentMaxNum = limits.
        maxTessellationControlTotalOutputComponents;

      deviceInfo.shaderStage.tesselationEvaluation.inputComponentMaxNum = limits.
        maxTessellationEvaluationInputComponents;
      deviceInfo.shaderStage.tesselationEvaluation.outputComponentMaxNum = limits.
        maxTessellationEvaluationOutputComponents;

      deviceInfo.shaderStage.geometry.invocationMaxNum = limits.maxGeometryShaderInvocations;
      deviceInfo.shaderStage.geometry.inputComponentMaxNum = limits.maxGeometryInputComponents;
      deviceInfo.shaderStage.geometry.outputComponentMaxNum = limits.maxGeometryOutputComponents;
      deviceInfo.shaderStage.geometry.outputVertexMaxNum = limits.maxGeometryOutputVertices;
      deviceInfo.shaderStage.geometry.totalOutputComponentMaxNum = limits.maxGeometryTotalOutputComponents;

      deviceInfo.shaderStage.fragment.inputComponentMaxNum = limits.maxFragmentInputComponents;
      deviceInfo.shaderStage.fragment.attachmentMaxNum = limits.maxFragmentOutputAttachments;
      deviceInfo.shaderStage.fragment.dualSourceAttachmentMaxNum = limits.maxFragmentDualSrcAttachments;

      deviceInfo.shaderStage.compute.dispatchMaxDim[0] = limits.maxComputeWorkGroupCount[0];
      deviceInfo.shaderStage.compute.dispatchMaxDim[1] = limits.maxComputeWorkGroupCount[1];
      deviceInfo.shaderStage.compute.dispatchMaxDim[2] = limits.maxComputeWorkGroupCount[2];
      deviceInfo.shaderStage.compute.workGroupInvocationMaxNum = limits.maxComputeWorkGroupInvocations;
      deviceInfo.shaderStage.compute.workGroupMaxDim[0] = limits.maxComputeWorkGroupSize[0];
      deviceInfo.shaderStage.compute.workGroupMaxDim[1] = limits.maxComputeWorkGroupSize[1];
      deviceInfo.shaderStage.compute.workGroupMaxDim[2] = limits.maxComputeWorkGroupSize[2];
      deviceInfo.shaderStage.compute.sharedMemoryMaxSize = limits.maxComputeSharedMemorySize;

      deviceInfo.shaderStage.task.dispatchWorkGroupMaxNum = meshShaderProps.maxTaskWorkGroupTotalCount;
      deviceInfo.shaderStage.task.dispatchMaxDim[0] = meshShaderProps.maxTaskWorkGroupCount[0];
      deviceInfo.shaderStage.task.dispatchMaxDim[1] = meshShaderProps.maxTaskWorkGroupCount[1];
      deviceInfo.shaderStage.task.dispatchMaxDim[2] = meshShaderProps.maxTaskWorkGroupCount[2];
      deviceInfo.shaderStage.task.workGroupInvocationMaxNum = meshShaderProps.maxTaskWorkGroupInvocations;
      deviceInfo.shaderStage.task.workGroupMaxDim[0] = meshShaderProps.maxTaskWorkGroupSize[0];
      deviceInfo.shaderStage.task.workGroupMaxDim[1] = meshShaderProps.maxTaskWorkGroupSize[1];
      deviceInfo.shaderStage.task.workGroupMaxDim[2] = meshShaderProps.maxTaskWorkGroupSize[2];
      deviceInfo.shaderStage.task.sharedMemoryMaxSize = meshShaderProps.maxTaskSharedMemorySize;
      deviceInfo.shaderStage.task.payloadMaxSize = meshShaderProps.maxTaskPayloadSize;

      deviceInfo.shaderStage.mesh.dispatchWorkGroupMaxNum = meshShaderProps.maxMeshWorkGroupTotalCount;
      deviceInfo.shaderStage.mesh.dispatchMaxDim[0] = meshShaderProps.maxMeshWorkGroupCount[0];
      deviceInfo.shaderStage.mesh.dispatchMaxDim[1] = meshShaderProps.maxMeshWorkGroupCount[1];
      deviceInfo.shaderStage.mesh.dispatchMaxDim[2] = meshShaderProps.maxMeshWorkGroupCount[2];
      deviceInfo.shaderStage.mesh.workGroupInvocationMaxNum = meshShaderProps.maxMeshWorkGroupInvocations;
      deviceInfo.shaderStage.mesh.workGroupMaxDim[0] = meshShaderProps.maxMeshWorkGroupSize[0];
      deviceInfo.shaderStage.mesh.workGroupMaxDim[1] = meshShaderProps.maxMeshWorkGroupSize[1];
      deviceInfo.shaderStage.mesh.workGroupMaxDim[2] = meshShaderProps.maxMeshWorkGroupSize[2];
      deviceInfo.shaderStage.mesh.sharedMemoryMaxSize = meshShaderProps.maxMeshSharedMemorySize;
      deviceInfo.shaderStage.mesh.outputVerticesMaxNum = meshShaderProps.maxMeshOutputVertices;
      deviceInfo.shaderStage.mesh.outputPrimitiveMaxNum = meshShaderProps.maxMeshOutputPrimitives;
      deviceInfo.shaderStage.mesh.outputComponentMaxNum = meshShaderProps.maxMeshOutputComponents;

      deviceInfo.shaderStage.rayTracing.shaderGroupIdentifierSize = rayTracingPipelineProps.shaderGroupHandleSize;
      deviceInfo.shaderStage.rayTracing.shaderBindingTableMaxStride = rayTracingPipelineProps.maxShaderGroupStride;
      deviceInfo.shaderStage.rayTracing.recursionMaxDepth = rayTracingPipelineProps.maxRayRecursionDepth;

      deviceInfo.accelerationStructure.primitiveMaxNum = accelerationStructureProps.maxPrimitiveCount;
      deviceInfo.accelerationStructure.geometryMaxNum = accelerationStructureProps.maxGeometryCount;
      deviceInfo.accelerationStructure.instanceMaxNum = accelerationStructureProps.maxInstanceCount;
      deviceInfo.accelerationStructure.micromapSubdivisionMaxLevel = opacityMicromapProps.
        maxOpacity2StateSubdivisionLevel;

      deviceInfo.wave.laneMinNum = deviceProperties13.minSubgroupSize;
      deviceInfo.wave.laneMaxNum = deviceProperties13.maxSubgroupSize;

      deviceInfo.wave.derivativeOpsStages = StageBits::FragmentShader;
      if (computeShaderDerivativesFeatures.computeDerivativeGroupQuads || computeShaderDerivativesFeatures.
        computeDerivativeGroupLinear)
        deviceInfo.wave.derivativeOpsStages |= StageBits::ComputeShader;
      if (computeShaderDerivativesProps.meshAndTaskShaderDerivatives)
        deviceInfo.wave.derivativeOpsStages |= StageBits::TaskShader | StageBits::MeshShader;

      if (deviceProperties11.subgroupSupportedOperations & VK_SUBGROUP_FEATURE_QUAD_BIT)
        deviceInfo.wave.quadOpsStages = deviceProperties11.subgroupQuadOperationsInAllStages
                                          ? StageBits::AllShaders
                                          : (StageBits::FragmentShader | StageBits::ComputeShader);

      if (deviceProperties11.subgroupSupportedStages & VK_SHADER_STAGE_VERTEX_BIT)
        deviceInfo.wave.waveOpsStages |= StageBits::VertexShader;
      if (deviceProperties11.subgroupSupportedStages & VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT)
        deviceInfo.wave.waveOpsStages |= StageBits::TessellationControlShader;
      if (deviceProperties11.subgroupSupportedStages & VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)
        deviceInfo.wave.waveOpsStages |= StageBits::TessellationEvaluationShader;
      if (deviceProperties11.subgroupSupportedStages & VK_SHADER_STAGE_GEOMETRY_BIT)
        deviceInfo.wave.waveOpsStages |= StageBits::GeometryShader;
      if (deviceProperties11.subgroupSupportedStages & VK_SHADER_STAGE_FRAGMENT_BIT)
        deviceInfo.wave.waveOpsStages |= StageBits::FragmentShader;
      if (deviceProperties11.subgroupSupportedStages & VK_SHADER_STAGE_COMPUTE_BIT)
        deviceInfo.wave.waveOpsStages |= StageBits::ComputeShader;
      if (deviceProperties11.subgroupSupportedStages & VK_SHADER_STAGE_RAYGEN_BIT_KHR)
        deviceInfo.wave.waveOpsStages |= StageBits::RayGenShader;
      if (deviceProperties11.subgroupSupportedStages & VK_SHADER_STAGE_ANY_HIT_BIT_KHR)
        deviceInfo.wave.waveOpsStages |= StageBits::AnyHitShader;
      if (deviceProperties11.subgroupSupportedStages & VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR)
        deviceInfo.wave.waveOpsStages |= StageBits::ClosestHitShader;
      if (deviceProperties11.subgroupSupportedStages & VK_SHADER_STAGE_MISS_BIT_KHR)
        deviceInfo.wave.waveOpsStages |= StageBits::MissShader;
      if (deviceProperties11.subgroupSupportedStages & VK_SHADER_STAGE_INTERSECTION_BIT_KHR)
        deviceInfo.wave.waveOpsStages |= StageBits::IntersectionShader;
      if (deviceProperties11.subgroupSupportedStages & VK_SHADER_STAGE_CALLABLE_BIT_KHR)
        deviceInfo.wave.waveOpsStages |= StageBits::CallableShader;
      if (deviceProperties11.subgroupSupportedStages & VK_SHADER_STAGE_TASK_BIT_EXT)
        deviceInfo.wave.waveOpsStages |= StageBits::TaskShader;
      if (deviceProperties11.subgroupSupportedStages & VK_SHADER_STAGE_MESH_BIT_EXT)
        deviceInfo.wave.waveOpsStages |= StageBits::MeshShader;

      deviceInfo.other.timestampFrequencyHz = uint64_t(1e9 / double(limits.timestampPeriod) + 0.5);
      deviceInfo.other.drawIndirectMaxNum = limits.maxDrawIndirectCount;
      deviceInfo.other.samplerLodBiasMax = limits.maxSamplerLodBias;
      deviceInfo.other.samplerAnisotropyMax = limits.maxSamplerAnisotropy;
      deviceInfo.other.texelOffsetMin = (int8_t)limits.minTexelOffset;
      deviceInfo.other.texelOffsetMax = (uint8_t)limits.maxTexelOffset;
      deviceInfo.other.texelGatherOffsetMin = (int8_t)limits.minTexelGatherOffset;
      deviceInfo.other.texelGatherOffsetMax = (uint8_t)limits.maxTexelGatherOffset;
      deviceInfo.other.clipDistanceMaxNum = (uint8_t)limits.maxClipDistances;
      deviceInfo.other.cullDistanceMaxNum = (uint8_t)limits.maxCullDistances;
      deviceInfo.other.combinedClipAndCullDistanceMaxNum = (uint8_t)limits.maxCombinedClipAndCullDistances;
      deviceInfo.other.viewMaxNum = deviceFeatures11.multiview ? (uint8_t)deviceProperties11.maxMultiviewViewCount : 1;
      deviceInfo.other.shadingRateAttachmentTileSize = (uint8_t)fragmentShadingRateProps.
                                                                minFragmentShadingRateAttachmentTexelSize.width;

      if (extensionSupported(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME, enabledDeviceExtensions)) {
        deviceInfo.tiers.conservativeRaster = 1;
        if (conservativeRasterizationProps.primitiveOverestimationSize < 1.0f / 2.0f && conservativeRasterizationProps.
          degenerateTrianglesRasterized)
          deviceInfo.tiers.conservativeRaster = 2;
        if (conservativeRasterizationProps.primitiveOverestimationSize <= 1.0 / 256.0f && conservativeRasterizationProps
          .degenerateTrianglesRasterized)
          deviceInfo.tiers.conservativeRaster = 3;
      }

      if (extensionSupported(VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME, enabledDeviceExtensions)) {
        constexpr VkSampleCountFlags allSampleCounts = VK_SAMPLE_COUNT_1_BIT | VK_SAMPLE_COUNT_2_BIT |
          VK_SAMPLE_COUNT_4_BIT | VK_SAMPLE_COUNT_8_BIT | VK_SAMPLE_COUNT_16_BIT;
        if (sampleLocationsProps.sampleLocationSampleCounts == allSampleCounts) // like in D3D12 spec
          deviceInfo.tiers.sampleLocations = 2;
      }

      if (accelerationStructureFeatures.accelerationStructure)
        deviceInfo.tiers.rayTracing = 1;
      if (deviceInfo.tiers.rayTracing == 1 && rayTracingPipelineFeatures.rayTracingPipelineTraceRaysIndirect &&
        rayQueryFeatures.rayQuery)
        deviceInfo.tiers.rayTracing = 2;
      if (deviceInfo.tiers.rayTracing == 2 && opacityMicromapFeatures.micromap)
        deviceInfo.tiers.rayTracing = 3;

      deviceInfo.tiers.shadingRate = fragmentShadingRateFeatures.pipelineFragmentShadingRate != 0 ? 1 : 0;
      if (deviceInfo.tiers.shadingRate && fragmentShadingRateFeatures.primitiveFragmentShadingRate &&
        fragmentShadingRateFeatures.attachmentFragmentShadingRate)
        deviceInfo.tiers.shadingRate = 2;

      // TODO: seems to be the best match
      deviceInfo.tiers.bindless = deviceFeatures12.descriptorIndexing ? 1 : 0;
      deviceInfo.tiers.resourceBinding = 2;
      deviceInfo.tiers.memory = 1;

      deviceInfo.features.swapChain = extensionSupported(VK_KHR_SWAPCHAIN_EXTENSION_NAME, enabledDeviceExtensions);
      deviceInfo.features.presentFromCompute = true;
      deviceInfo.features.waitableSwapChain = deviceInfo.features.swapChain != 0 && presentIdFeatures.presentId != 0 &&
        presentWaitFeatures.presentWait != 0;
      deviceInfo.features.resizableSwapChain = deviceInfo.features.swapChain != 0 && this->deviceFeatures.
        swapChainMaintenance1 != 0;
      deviceInfo.features.layerBasedMultiview = deviceFeatures11.multiview;
      deviceInfo.features.textureCompressionBC = deviceFeatures.features.textureCompressionBC;
      deviceInfo.features.textureCompressionETC2 = deviceFeatures.features.textureCompressionETC2;
      deviceInfo.features.textureCompressionASTC = deviceFeatures.features.textureCompressionASTC_LDR;
      deviceInfo.features.shaderBytecodeSPIRV = true;
      deviceInfo.features.occlusion = true;
      deviceInfo.features.timestamp = isTimestampSupported[
        static_cast<size_t>(QueueType::Graphics)] || isTimestampSupported[static_cast<size_t>(QueueType::Compute)
      ];
      deviceInfo.features.timestampCopyQueue = isTimestampSupported[static_cast<size_t>(QueueType::Copy)];
      deviceInfo.features.calibratedTimestamps = extensionSupported(
        VK_EXT_CALIBRATED_TIMESTAMPS_EXTENSION_NAME, enabledDeviceExtensions);
      deviceInfo.features.additionalShadingRates = fragmentShadingRateProps.maxFragmentSize.height > 2 ||
        fragmentShadingRateProps.maxFragmentSize.width > 2;
      deviceInfo.features.sumShadingRateCombiner = deviceInfo.tiers.shadingRate != 0;
      deviceInfo.features.regionResolve = true;
      deviceInfo.features.resolveOpMinMax = this->deviceFeatures.maintenance10 && this->deviceFeatures.copyCommands2;
      // TODO: it's "all or nothing", without it "min/max" resolve is supported only in a render pass
      deviceInfo.features.pipelineCache = true;
      deviceInfo.features.pipelineCacheControl = deviceFeatures13.pipelineCreationCacheControl;
      deviceInfo.features.getMemoryDesc2 = this->deviceFeatures.maintenance4;
      deviceInfo.features.enhancedBarriers = true;
      deviceInfo.features.tessellationShader = deviceFeatures.features.tessellationShader != 0;
      deviceInfo.features.geometryShader = deviceFeatures.features.geometryShader != 0;
      deviceInfo.features.meshShader = meshShaderFeatures.meshShader != 0 && meshShaderFeatures.taskShader != 0;
      deviceInfo.features.lowLatency = this->deviceFeatures.presentId != 0 && extensionSupported(
        VK_NV_LOW_LATENCY_2_EXTENSION_NAME, enabledDeviceExtensions);
      deviceInfo.features.componentSwizzle = true;
      deviceInfo.features.independentFrontAndBackStencilReferenceAndMasks = true;
      deviceInfo.features.filterOpMinMax = deviceFeatures12.samplerFilterMinmax;
      deviceInfo.features.logicOp = deviceFeatures.features.logicOp;
      deviceInfo.features.depthBoundsTest = deviceFeatures.features.depthBounds;
      deviceInfo.features.drawIndirectCount = deviceFeatures12.drawIndirectCount;
      deviceInfo.features.lineSmoothing = deviceFeatures14.smoothLines;
      deviceInfo.features.meshShaderPipelineStats = meshShaderFeatures.meshShaderQueries == VK_TRUE;
      deviceInfo.features.dynamicDepthBias = true;
      deviceInfo.features.viewportOriginBottomLeft = true;
      deviceInfo.features.pipelineStatistics = deviceFeatures.features.pipelineStatisticsQuery;
      deviceInfo.features.rootConstantsOffset = true;
      deviceInfo.features.nonConstantBufferRootDescriptorOffset = true;
      deviceInfo.features.mutableDescriptorType = mutableDescriptorTypeFeatures.mutableDescriptorType;
      deviceInfo.features.extendedDynamicState = extendedDynamicStateFeatures.extendedDynamicState;
      deviceInfo.features.unifiedTextureLayouts = unifiedImageLayoutsFeatures.unifiedImageLayouts;

      deviceInfo.shaderFeatures.nativeI8 = deviceFeatures12.shaderInt8;
      deviceInfo.shaderFeatures.nativeI16 = deviceFeatures.features.shaderInt16;
      deviceInfo.shaderFeatures.nativeF16 = deviceFeatures12.shaderFloat16;
      deviceInfo.shaderFeatures.nativeI64 = deviceFeatures.features.shaderInt64;
      deviceInfo.shaderFeatures.nativeF64 = deviceFeatures.features.shaderFloat64;
      deviceInfo.shaderFeatures.atomicsF16 = (shaderAtomicFloat2Features.shaderBufferFloat16Atomics ||
                                               shaderAtomicFloat2Features.shaderSharedFloat16Atomics)
                                               ? true
                                               : false;
      deviceInfo.shaderFeatures.atomicsF32 = (shaderAtomicFloatFeatures.shaderBufferFloat32Atomics ||
                                               shaderAtomicFloatFeatures.shaderSharedFloat32Atomics)
                                               ? true
                                               : false;
      deviceInfo.shaderFeatures.atomicsI64 = (deviceFeatures12.shaderBufferInt64Atomics || deviceFeatures12.
                                               shaderSharedInt64Atomics)
                                               ? true
                                               : false;
      deviceInfo.shaderFeatures.atomicsF64 = (shaderAtomicFloatFeatures.shaderBufferFloat64Atomics ||
                                               shaderAtomicFloatFeatures.shaderSharedFloat64Atomics)
                                               ? true
                                               : false;

      deviceInfo.shaderFeatures.storageReadWithoutFormat = deviceFeatures.features.shaderStorageImageReadWithoutFormat;
      deviceInfo.shaderFeatures.storageWriteWithoutFormat = deviceFeatures.features.
                                                                           shaderStorageImageWriteWithoutFormat;

      deviceInfo.shaderFeatures.waveQuery = (deviceProperties11.subgroupSupportedOperations &
                                              VK_SUBGROUP_FEATURE_BASIC_BIT)
                                              ? true
                                              : false;
      deviceInfo.shaderFeatures.waveVote = (deviceProperties11.subgroupSupportedOperations &
                                             VK_SUBGROUP_FEATURE_VOTE_BIT)
                                             ? true
                                             : false;
      deviceInfo.shaderFeatures.waveShuffle = (deviceProperties11.subgroupSupportedOperations &
                                                VK_SUBGROUP_FEATURE_SHUFFLE_BIT)
                                                ? true
                                                : false;
      deviceInfo.shaderFeatures.waveArithmetic = (deviceProperties11.subgroupSupportedOperations &
                                                   VK_SUBGROUP_FEATURE_ARITHMETIC_BIT)
                                                   ? true
                                                   : false;
      deviceInfo.shaderFeatures.waveReduction = (deviceProperties11.subgroupSupportedOperations &
                                                  VK_SUBGROUP_FEATURE_BALLOT_BIT)
                                                  ? true
                                                  : false;
      deviceInfo.shaderFeatures.waveQuad = (deviceProperties11.subgroupSupportedOperations &
                                             VK_SUBGROUP_FEATURE_QUAD_BIT)
                                             ? true
                                             : false;

      deviceInfo.shaderFeatures.viewportIndex = deviceFeatures12.shaderOutputViewportIndex;
      deviceInfo.shaderFeatures.layerIndex = deviceFeatures12.shaderOutputLayer;
      deviceInfo.shaderFeatures.unnormalizedCoordinates = true;
      deviceInfo.shaderFeatures.clock = (shaderClockFeatures.shaderDeviceClock || shaderClockFeatures.
                                          shaderSubgroupClock)
                                          ? true
                                          : false;
      deviceInfo.shaderFeatures.rasterizedOrderedView = fragmentShaderInterlockFeatures.fragmentShaderPixelInterlock !=
        0 &&
        fragmentShaderInterlockFeatures.fragmentShaderSampleInterlock != 0;
      deviceInfo.shaderFeatures.barycentric = fragmentShaderBarycentricFeatures.fragmentShaderBarycentric;
      deviceInfo.shaderFeatures.rayTracingPositionFetch = rayTracingPositionFetchFeatures.rayTracingPositionFetch;
      deviceInfo.shaderFeatures.integerDotProduct = deviceFeatures13.shaderIntegerDotProduct;
      deviceInfo.shaderFeatures.inputAttachments = deviceFeatures14.dynamicRenderingLocalRead || !deviceFeatures13.
        dynamicRendering;

      // legacy render passes support "input attachments"
      deviceInfo.shaderFeatures.drawParameters = deviceFeatures11.shaderDrawParameters ? true : false;
      // TODO: emulation is not implemented, because >99% devices support it!
      deviceInfo.shaderFeatures.drawIndex = deviceInfo.shaderFeatures.drawParameters;

      // // Estimate shader model last since it depends on many "deviceInfo" fields
      // // Based on https://docs.vulkan.org/guide/latest/hlsl.html#_shader_model_coverage // TODO: code below needs to be improved
      // deviceInfo.shaderModel = ENGINE_SHADER_MODEL(5, 1);
      // if (deviceInfo.shaderFeatures.nativeI64)
      //   deviceInfo.shaderModel = ENGINE_SHADER_MODEL(6, 0);
      // if (deviceInfo.other.viewMaxNum > 1 || deviceInfo.shaderFeatures.barycentric)
      //   deviceInfo.shaderModel = ENGINE_SHADER_MODEL(6, 1);
      // if (deviceInfo.shaderFeatures.nativeF16 || deviceInfo.shaderFeatures.nativeI16)
      //   deviceInfo.shaderModel = ENGINE_SHADER_MODEL(6, 2);
      // if (deviceInfo.tiers.rayTracing)
      //   deviceInfo.shaderModel = ENGINE_SHADER_MODEL(6, 3);
      // if (deviceInfo.tiers.shadingRate >= 2)
      //   deviceInfo.shaderModel = ENGINE_SHADER_MODEL(6, 4);
      // if (deviceInfo.features.meshShader || deviceInfo.tiers.rayTracing >= 2)
      //   deviceInfo.shaderModel = ENGINE_SHADER_MODEL(6, 5);
      // // TODO: "deviceInfo.features.mutableDescriptorType" is an optional feature, despite that it's needed to emulate SM 6.6 "ultimate" bindless
      // if (deviceInfo.shaderFeatures.atomicsI64)
      //   deviceInfo.shaderModel = ENGINE_SHADER_MODEL(6, 6);
      // if (deviceFeatures.features.shaderStorageImageMultisample)
      //   deviceInfo.shaderModel = ENGINE_SHADER_MODEL(6, 7);
      // // TODO: add SM 6.8+ detection
    }

    // Create VMA
    VmaVulkanFunctions vulkanFunctions = {};
    VmaAllocatorCreateInfo allocatorCreateInfo = {};

    allocatorCreateInfo.vulkanApiVersion = VK_MAKE_API_VERSION(0, 1, minorVersion, 0);
    allocatorCreateInfo.physicalDevice = physicalDevice;
    allocatorCreateInfo.device = device;
    allocatorCreateInfo.instance = instance;
    allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;
    allocatorCreateInfo.pAllocationCallbacks = &vulkanAllocationCallbacks;
    allocatorCreateInfo.preferredLargeHeapBlockSize = 0; // = VMA_DEFAULT_LARGE_HEAP_BLOCK_SIZE
    VULKAN_CHECK(vmaImportVulkanFunctionsFromVolk(&allocatorCreateInfo, &vulkanFunctions));

    allocatorCreateInfo.flags = 0;
    if (this->deviceFeatures.memoryBudget)
      allocatorCreateInfo.flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
    if (this->deviceFeatures.deviceAddress)
      allocatorCreateInfo.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    if (this->deviceFeatures.memoryPriority)
      allocatorCreateInfo.flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_PRIORITY_BIT;
    if (this->deviceFeatures.maintenance4)
      allocatorCreateInfo.flags |= VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE4_BIT;
    if (this->deviceFeatures.maintenance5)
      allocatorCreateInfo.flags |= VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE5_BIT;

    VULKAN_CHECK(vmaCreateAllocator(&allocatorCreateInfo, &vmaAllocator));

    return Result::Success;
  }

  Result VulkanDevice::createInstance(
    bool validationLayerEnabled,
    const Vector<const char*> &enabledInstanceExtensions
  ) {
    Vector<const char*> layers(stdAllocator);

    // Instance layers
    {
      if (validationLayerEnabled)
        layers.push_back("VK_LAYER_KHRONOS_validation");

      uint32_t layerCount = 0;
      vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

      Vector<VkLayerProperties> supportedLayers(layerCount, stdAllocator);
      vkEnumerateInstanceLayerProperties(&layerCount, supportedLayers.data());

      for (size_t i = 0; i < layers.size(); i++) {
        bool found = false;
        for (uint32_t j = 0; j < layerCount && !found; j++) {
          if (strcmp(supportedLayers[j].layerName, layers[i]) == 0)
            found = true;
        }

        if (!found)
          layers.erase(layers.begin() + i--);
      }
    }

    VkApplicationInfo applicationInfo{};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.apiVersion = VK_API_VERSION_1_4;

    constexpr std::array enabledValidationFeatures{
      // TODO: add VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT?
      VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT,
    };

    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

#ifdef ENGINE_PLATFORM_APPLE
    instanceCreateInfo.flags = (VkInstanceCreateFlags)VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
#endif
    instanceCreateInfo.pApplicationInfo = &applicationInfo;
    instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
    instanceCreateInfo.ppEnabledLayerNames = layers.data();
    instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(enabledInstanceExtensions.size());
    instanceCreateInfo.ppEnabledExtensionNames = enabledInstanceExtensions.data();

    VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo{};
    messengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    messengerCreateInfo.pUserData = this;
    messengerCreateInfo.pfnUserCallback = messageCallback;
    messengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
      | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
      | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
      | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    messengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
      | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
      | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

    instanceCreateInfo.pNext = &messengerCreateInfo;

    VkValidationFeaturesEXT validationFeatures{};
    validationFeatures.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
    validationFeatures.enabledValidationFeatureCount = enabledValidationFeatures.size();
    validationFeatures.pEnabledValidationFeatures = enabledValidationFeatures.data();

    if (validationLayerEnabled)
      messengerCreateInfo.pNext = &validationFeatures;

    VULKAN_CHECK(vkCreateInstance(&instanceCreateInfo, &vulkanAllocationCallbacks, &instance));
    volkLoadInstanceOnly(instance);

    if (validationLayerEnabled) {
      VULKAN_CHECK(
        vkCreateDebugUtilsMessengerEXT(instance, &messengerCreateInfo, &vulkanAllocationCallbacks, &debugMessenger)
      );
    }

    return Result::Success;
  }

  void VulkanDevice::loadInstanceExtensions(Vector<const char*> &enabledExtensions) {
    // Query extensions
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    Vector<VkExtensionProperties> supportedExtensions(extensionCount, stdAllocator);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, supportedExtensions.data());

#ifdef ENGINE_DEBUG
    for (const VkExtensionProperties &extension: supportedExtensions) {
      LOG_CORE_TRACE(
        "Found supported instance extension on device: {} {}",
        extension.extensionName, extension.specVersion
      );
    }
#endif

    {
      uint32_t sdlExtensionCount = 0;
      char const *const*sdlExtensions = SDL_Vulkan_GetInstanceExtensions(&sdlExtensionCount);
      enabledExtensions.assign(sdlExtensions, sdlExtensions + sdlExtensionCount);
    }

#ifdef ENGINE_PLATFORM_APPLE
    // Mandatory
    enabledExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    enabledExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#endif

    // Optional
    addExtension(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME, enabledExtensions, supportedExtensions);
    addExtension(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME, enabledExtensions, supportedExtensions);
    addExtension(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME, enabledExtensions, supportedExtensions);
    addExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, enabledExtensions, supportedExtensions);

#ifdef ENGINE_DEBUG
    for (const auto &extension : enabledExtensions) {
      LOG_CORE_TRACE("Enabled instance extension on device: {}", extension);
    }
#endif
  }

  void VulkanDevice::loadDeviceExtensions(Vector<const char*> &enabledDeviceExtensions, bool disableRayTracing) {
    // Query extensions
    uint32_t extensionsCount = 0;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionsCount, nullptr);

    Vector<VkExtensionProperties> supportedExtensions(extensionsCount, stdAllocator);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionsCount, supportedExtensions.data());

#ifdef ENGINE_DEBUG
    for (const VkExtensionProperties &properties : supportedExtensions)
      LOG_CORE_TRACE("Found supported device extension: {} {}", properties.extensionName, properties.specVersion);
#endif

#ifdef ENGINE_PLATFORM_APPLE
    addExtension(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
#endif

    /* clang-format off */
    if (minorVersion < 3) {
      addExtension(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
      addExtension(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
      addExtension(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
      addExtension(VK_KHR_MAINTENANCE_4_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
      addExtension(VK_KHR_SHADER_INTEGER_DOT_PRODUCT_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
      addExtension(VK_EXT_IMAGE_ROBUSTNESS_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
      addExtension(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions); // TODO: there are 2 and 3 versions...
      addExtension(VK_EXT_SUBGROUP_SIZE_CONTROL_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
      addExtension(VK_EXT_PIPELINE_CREATION_CACHE_CONTROL_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
    }

    if (minorVersion < 4) {
      addExtension(VK_KHR_LINE_RASTERIZATION_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
      addExtension(VK_KHR_MAINTENANCE_5_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
      addExtension(VK_KHR_MAINTENANCE_6_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
      addExtension(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
      addExtension(VK_KHR_DYNAMIC_RENDERING_LOCAL_READ_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
      addExtension(VK_EXT_PIPELINE_ROBUSTNESS_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
    }

    if (!disableRayTracing) {
      addExtension(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
      addExtension(VK_KHR_RAY_QUERY_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
      addExtension(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
      addExtension(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
      addExtension(VK_KHR_RAY_TRACING_POSITION_FETCH_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
      addExtension(VK_EXT_OPACITY_MICROMAP_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
    }

    addExtension(VK_KHR_COMPUTE_SHADER_DERIVATIVES_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
    addExtension(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
    addExtension(VK_KHR_FRAGMENT_SHADER_BARYCENTRIC_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
    addExtension(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
    addExtension(VK_KHR_MAINTENANCE_7_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
    addExtension(VK_KHR_MAINTENANCE_8_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
    addExtension(VK_KHR_MAINTENANCE_9_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
    addExtension(VK_KHR_MAINTENANCE_10_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
    addExtension(VK_KHR_MAINTENANCE1_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
    addExtension(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
    addExtension(VK_KHR_PRESENT_ID_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
    addExtension(VK_KHR_PRESENT_WAIT_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
    addExtension(VK_KHR_SHADER_CLOCK_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
    addExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
    addExtension(VK_KHR_SWAPCHAIN_MUTABLE_FORMAT_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
    addExtension(VK_KHR_UNIFIED_IMAGE_LAYOUTS_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
    addExtension(VK_EXT_CALIBRATED_TIMESTAMPS_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions); // TODO: use KHR (currently coverage is lower)
    addExtension(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
    addExtension(VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
    addExtension(VK_EXT_FRAGMENT_SHADER_INTERLOCK_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
    addExtension(VK_EXT_IMAGE_SLICED_VIEW_OF_3D_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
    addExtension(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
    addExtension(VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
    addExtension(VK_EXT_MESH_SHADER_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
    addExtension(VK_EXT_PRESENT_MODE_FIFO_LATEST_READY_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
    addExtension(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions); // TODO: use KHR (currently coverage is lower)
    addExtension(VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
    addExtension(VK_EXT_SHADER_ATOMIC_FLOAT_2_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
    addExtension(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
    addExtension(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
    addExtension(VK_EXT_ZERO_INITIALIZE_DEVICE_MEMORY_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
    addExtension(VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
    addExtension(VK_NVX_BINARY_IMPORT_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
    addExtension(VK_NVX_IMAGE_VIEW_HANDLE_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
    addExtension(VK_NV_LOW_LATENCY_2_EXTENSION_NAME, enabledDeviceExtensions, supportedExtensions);
    /* clang-format on */
  }

#define UPDATE_TEXTURE_SUPPORT_BITS(required, bit) \
  if ((props3.optimalTilingFeatures & (required)) == (required)) \
    supportBits |= bit;

#define UPDATE_BUFFER_SUPPORT_BITS(required, bit) \
  if ((props3.bufferFeatures & (required)) == (required)) \
    supportBits |= bit;
  FormatSupportBits VulkanDevice::getFormatSupport(Format format) const {
    FormatSupportBits supportBits = FormatSupportBits::Unsupported;
    VkFormat vkFormat = formatToVulkanFormat(format);
    if (vkFormat == VK_FORMAT_UNDEFINED)
      return FormatSupportBits::Unsupported;

    VkFormatProperties3 props3 = {VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_3};
    VkFormatProperties2 props2 = {VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2, &props3};
    vkGetPhysicalDeviceFormatProperties2(physicalDevice, vkFormat, &props2);

    UPDATE_TEXTURE_SUPPORT_BITS(VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_BIT, FormatSupportBits::Texture);
    UPDATE_TEXTURE_SUPPORT_BITS(VK_FORMAT_FEATURE_2_STORAGE_IMAGE_BIT, FormatSupportBits::StorageTexture);
    UPDATE_TEXTURE_SUPPORT_BITS(VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BIT, FormatSupportBits::ColorAttachment);
    UPDATE_TEXTURE_SUPPORT_BITS(VK_FORMAT_FEATURE_2_DEPTH_STENCIL_ATTACHMENT_BIT,
                                FormatSupportBits::DepthStencilAttachment);
    UPDATE_TEXTURE_SUPPORT_BITS(VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BLEND_BIT, FormatSupportBits::Blend);
    UPDATE_TEXTURE_SUPPORT_BITS(VK_FORMAT_FEATURE_2_STORAGE_IMAGE_ATOMIC_BIT, FormatSupportBits::StorageTextureAtomics);

    UPDATE_BUFFER_SUPPORT_BITS(VK_FORMAT_FEATURE_2_UNIFORM_TEXEL_BUFFER_BIT, FormatSupportBits::Buffer);
    UPDATE_BUFFER_SUPPORT_BITS(VK_FORMAT_FEATURE_2_STORAGE_TEXEL_BUFFER_BIT, FormatSupportBits::StorageBuffer);
    UPDATE_BUFFER_SUPPORT_BITS(VK_FORMAT_FEATURE_2_VERTEX_BUFFER_BIT, FormatSupportBits::VertexBuffer);
    UPDATE_BUFFER_SUPPORT_BITS(VK_FORMAT_FEATURE_2_STORAGE_TEXEL_BUFFER_ATOMIC_BIT,
                               FormatSupportBits::StorageBufferAtomics);

    if (supportBits & FormatSupportBits::ColorAttachment)
      supportBits |= FormatSupportBits::MultiSampleResolve;
    if ((supportBits & FormatSupportBits::DepthStencilAttachment) && deviceFeatures.maintenance10)
      supportBits |= FormatSupportBits::MultiSampleResolve;

    if ((props3.optimalTilingFeatures | props3.bufferFeatures) & VK_FORMAT_FEATURE_2_STORAGE_READ_WITHOUT_FORMAT_BIT)
      supportBits |= FormatSupportBits::StorageReadWithoutFormat;

    if ((props3.optimalTilingFeatures | props3.bufferFeatures) & VK_FORMAT_FEATURE_2_STORAGE_WRITE_WITHOUT_FORMAT_BIT)
      supportBits |= FormatSupportBits::StorageReadWithoutFormat;

    VkPhysicalDeviceImageFormatInfo2 imageInfo = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2};
    imageInfo.format = vkFormat;
    imageInfo.type = VK_IMAGE_TYPE_2D;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.flags = 0; // TODO: kinda needed, but unknown here

    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (supportBits & FormatSupportBits::Texture)
      imageInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
    if (supportBits & FormatSupportBits::DepthStencilAttachment)
      imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    if (supportBits & FormatSupportBits::ColorAttachment)
      imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkImageFormatProperties2 imageProps = {VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2};
    vkGetPhysicalDeviceImageFormatProperties2(physicalDevice, &imageInfo, &imageProps);

    if (imageProps.imageFormatProperties.sampleCounts & VK_SAMPLE_COUNT_2_BIT)
      supportBits |= FormatSupportBits::Multisample2X;
    if (imageProps.imageFormatProperties.sampleCounts & VK_SAMPLE_COUNT_4_BIT)
      supportBits |= FormatSupportBits::MultiSample4X;
    if (imageProps.imageFormatProperties.sampleCounts & VK_SAMPLE_COUNT_8_BIT)
      supportBits |= FormatSupportBits::MultiSample8X;

    return supportBits;
  }
#undef UPDATE_TEXTURE_SUPPORT_BITS
#undef UPDATE_BUFFER_SUPPORT_BITS

  bool VulkanDevice::getMemoryInfo(
    MemoryLocation memoryLocation,
    const VkMemoryRequirements &memoryRequirements,
    const VkMemoryDedicatedRequirements &memoryDedicatedRequirements,
    MemoryInfo &memoryInfo
  ) const {
    VkMemoryPropertyFlags neededFlags = 0; // must have
    VkMemoryPropertyFlags undesiredFlags = 0; // have higher priority than desired
    VkMemoryPropertyFlags desiredFlags = 0; // nice to have
    if (memoryLocation == MemoryLocation::Device) {
      neededFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
      undesiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    }
    else if (memoryLocation == MemoryLocation::DeviceUpload) {
      neededFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
      undesiredFlags = VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
      desiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    }
    else {
      neededFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
      undesiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
      desiredFlags = (memoryLocation == MemoryLocation::HostReadback ? VK_MEMORY_PROPERTY_HOST_CACHED_BIT : 0) |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    }

    memoryInfo = {};

    for (uint32_t phase = 0; phase < 4; phase++) {
      for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
        bool isSupported = memoryRequirements.memoryTypeBits & (1u << i);
        bool hasNeededFlags = (memoryProperties.memoryTypes[i].propertyFlags & neededFlags) == neededFlags;
        bool hasUndesiredFlags = undesiredFlags == 0
                                   ? false
                                   : (memoryProperties.memoryTypes[i].propertyFlags & undesiredFlags) == undesiredFlags;
        bool hasDesiredFlags = (memoryProperties.memoryTypes[i].propertyFlags & desiredFlags) == desiredFlags;

        bool isOK = isSupported && hasNeededFlags; // phase 3 - only needed
        if (phase == 0)
          isOK = isOK && !hasUndesiredFlags && hasDesiredFlags; // phase 0 - needed, undesired and desired
        else if (phase == 1)
          isOK = isOK && !hasUndesiredFlags; // phase 1 - needed, undesired
        else if (phase == 2)
          isOK = isOK && hasDesiredFlags; // phase 2 - needed and desired

        if (isOK) {
          VulkanMemoryTypeInfo memoryTypeInfo = {};
          memoryTypeInfo.index = static_cast<uint16_t>(i);
          memoryTypeInfo.location = memoryLocation;

          // "prefersDedicatedAllocation" seems to be "too soft" making more allocations "dedicated", "requiresDedicatedAllocation" better matches D3D12
          memoryTypeInfo.mustBeDedicated = memoryDedicatedRequirements.requiresDedicatedAllocation;

          memoryInfo.size = memoryRequirements.size;
          memoryInfo.alignment = static_cast<uint32_t>(memoryRequirements.alignment);
          memoryInfo.type = *reinterpret_cast<uint32_t*>(&memoryTypeInfo);
          memoryInfo.mustBeDedicated = memoryTypeInfo.mustBeDedicated;

          return true;
        }
      }
    }

    ENGINE_ASSERT(false, "Can't find suitable memory type");

    return false;
  }

  void VulkanDevice::addExtension(
    const char *extension,
    Vector<const char*> &enabledExtensions,
    const Vector<VkExtensionProperties> &supportedExtensions
  ) const {
    if (extensionSupported(extension, supportedExtensions))
      enabledExtensions.push_back(extension);
  }

  bool VulkanDevice::extensionSupported(
    const char *extension,
    const Vector<VkExtensionProperties> &supportedExtensions
  ) const {
    for (auto &supportedExtension : supportedExtensions) {
      if (!strcmp(extension, supportedExtension.extensionName))
        return true;
    }

    return false;
  }

  bool VulkanDevice::extensionSupported(
    const char *extension,
    const Vector<const char*> &supportedExtensions
  ) const {
    for (auto &supportedExtension : supportedExtensions) {
      if (!strcmp(extension, supportedExtension))
        return true;
    }

    return false;
  }
}
