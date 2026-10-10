// © 2021 NVIDIA Corporation

// Goal: wrapping native D3D12 objects into NRI objects

#pragma once

#define NRI_WRAPPER_D3D12_H 1

#include "device_creation.hpp"
#include "ray_tracing.hpp"

typedef int32_t DXGIFormat;

struct AGSContext;
struct ID3D12Heap;
struct ID3D12Fence;
struct ID3D12Device;
struct ID3D12Resource;
struct ID3D12CommandQueue;
struct ID3D12DescriptorHeap;
struct ID3D12CommandAllocator;
struct ID3D12GraphicsCommandList;

namespace Core::RHI {
  // A collection of queues of the same type
  struct QueueFamilyD3D12Desc {
    RHI_OPTIONAL ID3D12CommandQueue* const* d3d12Queues; // if not provided, will be created
    uint32_t queueNum;
    QueueType queueType;
  };

  struct DeviceCreationD3D12Desc {
    ID3D12Device* d3d12Device;
    const QueueFamilyD3D12Desc *queueFamilies;
    uint32_t queueFamilyNum;
    RHI_OPTIONAL AGSContext* agsContext;
    RHI_OPTIONAL CallbackInterface callbackInterface;
    RHI_OPTIONAL AllocationCallbacks allocationCallbacks;
    RHI_OPTIONAL uint32_t d3dShaderExtRegister;  // vendor specific shader extensions (default is "NRI_SHADER_EXT_REGISTER", space is always "0")
    RHI_OPTIONAL uint32_t d3dZeroBufferSize;     // no "memset" functionality in D3D, "CmdZeroBuffer" implemented via a bunch of copies (4 Mb by default)

    // Switches (disabled by default)
    bool enableNRIValidation;
    bool enableMemoryZeroInitialization;        // page-clears are fast, not enabled by default to match VK (the extension needed)

    // Switches (enabled by default)
    bool disableD3D12EnhancedBarriers;          // even if AgilitySDK is in use, some apps still use legacy barriers. It can be important for integrations
    bool disableNVAPIInitialization;            // at least NVAPI requires calling "NvAPI_Initialize" in DLL/EXE where the device is created
    };

    struct CommandBufferD3D12Desc {
    ID3D12GraphicsCommandList* d3d12CommandList;
    RHI_OPTIONAL ID3D12CommandAllocator* d3d12CommandAllocator; // needed only for "BeginCommandBuffer"
  };

  struct DescriptorPoolD3D12Desc {
    ID3D12DescriptorHeap* d3d12ResourceDescriptorHeap;
    ID3D12DescriptorHeap* d3d12SamplerDescriptorHeap;

    // Allocation limits (D3D12 unrelated, but must match expected usage)
    uint32_t descriptorSetMaxNum;
  };

  struct BufferD3D12Desc {
    ID3D12Resource* d3d12Resource;
    RHI_OPTIONAL const BufferDesc *desc;  // not all information can be retrieved from the resource if not provided
    RHI_OPTIONAL uint32_t structureStride;       // must be provided if used as a structured or raw buffer
  };

  struct TextureD3D12Desc {
    ID3D12Resource* d3d12Resource;
    RHI_OPTIONAL DXGIFormat format;              // must be provided "as a compatible typed format" if the resource is typeless
  };

  struct MemoryD3D12Desc {
    ID3D12Heap* d3d12Heap;
    uint64_t offset;
  };

  struct FenceD3D12Desc {
    ID3D12Fence* d3d12Fence;
  };

  struct AccelerationStructureD3D12Desc {
    ID3D12Resource* d3d12Resource;
    AccelerationStructureBits flags;

    // D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO
    uint64_t size;
    uint64_t buildScratchSize;
    uint64_t updateScratchSize;
  };

  // Threadsafe: yes
  struct WrapperD3D12Interface {
    Result (*CreateCommandBufferD3D12)            (Device &Device, const CommandBufferD3D12Desc &CommandBufferD3D12Desc, RHI_OUT CommandBuffer* &commandBuffer);
    Result (*CreateDescriptorPoolD3D12)           (Device &Device, const DescriptorPoolD3D12Desc &DescriptorPoolD3D12Desc, RHI_OUT DescriptorPool* &descriptorPool);
    Result (*CreateBufferD3D12)                   (Device &Device, const BufferD3D12Desc &BufferD3D12Desc, RHI_OUT Buffer* &buffer);
    Result (*CreateTextureD3D12)                  (Device &Device, const TextureD3D12Desc &TextureD3D12Desc, RHI_OUT Texture* &texture);
    Result (*CreateMemoryD3D12)                   (Device &Device, const MemoryD3D12Desc &MemoryD3D12Desc, RHI_OUT Memory* &memory);
    Result (*CreateFenceD3D12)                    (Device &Device, const FenceD3D12Desc &FenceD3D12Desc, RHI_OUT Fence* &fence);
    Result (*CreateAccelerationStructureD3D12)    (Device &Device, const AccelerationStructureD3D12Desc &AccelerationStructureD3D12Desc, RHI_OUT AccelerationStructure *&accelerationStructure);
  };

  Result nriCreateDeviceFromD3D12Device(const DeviceCreationD3D12Desc &deviceDesc, RHI_OUT Device *&device);
}