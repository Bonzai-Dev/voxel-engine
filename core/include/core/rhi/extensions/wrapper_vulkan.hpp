// © 2021 NVIDIA Corporation

// Goal: wrapping native VK objects into NRI objects

#pragma once

#define NRI_WRAPPER_VK_H 1

typedef void *VKHandle;
typedef int32_t VKEnum;
typedef uint32_t VKFlags;
typedef uint64_t VKNonDispatchableHandle;

namespace Core::RHI {
  struct AccelerationStructure;

  // A collection of queues of the same type
  struct QueueFamilyVKDesc {
    uint32_t queueNum;
    QueueType queueType;
    uint32_t familyIndex;
  };

  struct DeviceCreationVKDesc {
    RHI_OPTIONAL CallbackInterface callbackInterface;
    RHI_OPTIONAL AllocationCallbacks allocationCallbacks;
    RHI_OPTIONAL const char *libraryPath;
    VKBindingOffsets vkBindingOffsets;
    VKExtensions vkExtensions; // enabled
    VKHandle vkInstance;
    VKHandle vkDevice;
    VKHandle vkPhysicalDevice;
    const QueueFamilyVKDesc *queueFamilies;
    uint32_t queueFamilyNum;
    uint8_t minorVersion; // >= 2

    // Switches (disabled by default)
    bool enableNRIValidation;
    bool enableMemoryZeroInitialization; // page-clears are fast, but memory is not cleared by default in VK
  };

  struct CommandAllocatorVKDesc {
    VKNonDispatchableHandle vkCommandPool;
    QueueType queueType;
  };

  struct CommandBufferVKDesc {
    VKHandle vkCommandBuffer;
    QueueType queueType;
  };

  struct DescriptorPoolVKDesc {
    VKNonDispatchableHandle vkDescriptorPool;
    uint32_t descriptorSetMaxNum;
  };

  struct BufferVKDesc {
    VKNonDispatchableHandle vkBuffer;
    uint64_t size;
    RHI_OPTIONAL uint32_t structureStride; // must be provided if used as a structured or raw buffer
    RHI_OPTIONAL uint8_t *mappedMemory; // must be provided if the underlying memory is mapped
    RHI_OPTIONAL VKNonDispatchableHandle vkDeviceMemory;
    // must be provided *only* if the mapped memory exists and *not* HOST_COHERENT
    RHI_OPTIONAL uint64_t deviceAddress; // must be provided for ray tracing
  };

  struct TextureVKDesc {
    VKNonDispatchableHandle vkImage;
    VKEnum vkFormat;
    VKEnum vkImageType;
    VKFlags vkImageUsageFlags;
    Dim_t width;
    Dim_t height;
    Dim_t depth;
    Dim_t mipNum;
    Dim_t layerNum;
    Sample_t sampleNum;
  };

  struct MemoryVKDesc {
    VKNonDispatchableHandle vkDeviceMemory;
    RHI_OPTIONAL uint64_t offset;
    RHI_OPTIONAL void *mappedMemory; // at "offset"
    uint64_t size;
    uint32_t memoryTypeIndex;
  };

  struct PipelineVKDesc {
    VKNonDispatchableHandle vkPipeline;
    VKEnum vkPipelineBindPoint;
  };

  struct QueryPoolVKDesc {
    VKNonDispatchableHandle vkQueryPool;
    VKEnum vkQueryType;
  };

  struct FenceVKDesc {
    VKNonDispatchableHandle vkTimelineSemaphore;
  };

  struct AccelerationStructureVKDesc {
    VKNonDispatchableHandle vkAccelerationStructure;
    VKNonDispatchableHandle vkBuffer;
    uint64_t bufferSize;
    uint64_t buildScratchSize;
    uint64_t updateScratchSize;
    AccelerationStructureBits flags;
  };

  /*clang-format off*/
  // Threadsafe: yes
  struct WrapperVKInterface {
    Result      (*CreateCommandAllocatorVK)        (Device &Device, const CommandAllocatorVKDesc &CommandAllocatorVKDesc, RHI_OUT CommandAllocator *&commandAllocator);
    Result      (*CreateCommandBufferVK)           (Device &Device, const CommandBufferVKDesc &CommandBufferVKDesc, RHI_OUT CommandBuffer *&commandBuffer);
    Result      (*CreateDescriptorPoolVK)          (Device &Device, const DescriptorPoolVKDesc &DescriptorPoolVKDesc, RHI_OUT DescriptorPool *&descriptorPool);
    Result      (*CreateBufferVK)                  (Device &Device, const BufferVKDesc &BufferVKDesc, RHI_OUT Buffer *&buffer);
    Result      (*CreateTextureVK)                 (Device &Device, const TextureVKDesc &TextureVKDesc, RHI_OUT Texture *&texture);
    Result      (*CreateMemoryVK)                  (Device &Device, const MemoryVKDesc &MemoryVKDesc, RHI_OUT Memory *&memory);
    Result      (*CreatePipelineVK)                (Device &Device, const PipelineVKDesc &PipelineVKDesc, RHI_OUT Pipeline *&pipeline);
    Result      (*CreateQueryPoolVK)               (Device &Device, const QueryPoolVKDesc &QueryPoolVKDesc, RHI_OUT QueryPool *&queryPool);
    Result      (*CreateFenceVK)                   (Device &Device, const FenceVKDesc &FenceVKDesc, RHI_OUT Fence *&fence);
    Result      (*CreateAccelerationStructureVK)   (Device &Device, const AccelerationStructureVKDesc &AccelerationStructureVKDesc, RHI_OUT AccelerationStructure *&accelerationStructure);

    uint32_t    (*GetQueueFamilyIndexVK)           (const Queue &Queue);
    VKHandle    (*GetPhysicalDeviceVK)             (const Device &Device);
    VKHandle    (*GetInstanceVK)                   (const Device &Device);
    void*       (*GetInstanceProcAddrVK)           (const Device &Device);
    void*       (*GetDeviceProcAddrVK)             (const Device &Device);
  };
  /*clang-format on*/

  Result nriCreateDeviceFromVKDevice(const DeviceCreationVKDesc &deviceDesc, RHI_OUT Device *&device);
}
