// © 2021 NVIDIA Corporation

#pragma once

#include <stddef.h>
#include <stdint.h>

#include "../core.hpp"

#if defined(_WIN32)
#define NRI_CALL __stdcall
#else
#define NRI_CALL
#endif

#ifndef NRI_API
#if defined(__cplusplus)
#define NRI_API extern "C"
#else
#define NRI_API extern
#endif
#endif

#ifdef __cplusplus
#if !defined(NRI_FORCE_C)
#define NRI_CPP
#endif
#else
#include <stdbool.h>
#endif

// #include "NRIMacro.h"

// Tips:
// - designated initializers are highly recommended!
// - always zero initialize structs via "{}" if designated initializers are not used (at least to honor "NriOptional")
// - documentation is embedded (more details can be requested by creating a GitHub issue)
// - data types are grouped into collapsible logical blocks via "#pragma region"
// - in function declarations "NriRef" implies a valid object, "NriPtr" means "NULL" is allowed

namespace Core::RHI {
  // Entities
  class Fence {};

  // a synchronization primitive that can be used to insert a dependency between queue operations or between a queue operation and the host
  class Queue {}; // a logical queue, providing access to a HW queue
  class Memory {}; // a memory blob allocated on DEVICE or HOST
  class Buffer {}; // a buffer object: linear arrays of data
  class Device {}; // a logical device
  class Texture {}; // a texture object: multidimensional arrays of data
  class Pipeline {}; // a collection of state needed for rendering: shaders + fixed
  class SwapChain {}; // an array of presentable images that are associated with a surface
  class QueryPool {}; // a collection of queries of the same type
  class Descriptor {}; // a handle or pointer to a resource (potentially with a header)
  class CommandBuffer {};
  // used to record commands which can be subsequently submitted to a device queue for execution (aka command list)
  class DescriptorSet {}; // a continuous set of descriptors
  class DescriptorPool {}; // maintains a pool of descriptors, descriptor sets are allocated from
  class PipelineLayout {};
  // determines the interface between shader stages and shader resources (aka root signature)
  class PipelineCache {};
  // a persistent cache of compiled pipeline state objects (PSOs) to accelerate subsequent PSO creations
  class CommandAllocator {}; // an object that command buffer memory is allocated from

  // Basic types
  typedef uint8_t Nri(Sample_t);
  typedef uint16_t Nri(Dim_t);
  typedef void Nri(Object);

  struct Uid_t {
    uint64_t low;
    uint64_t high;
  };

  struct Dim2_t {
    Dim_t w, h;
  };

  struct Float2_t {
    float x, y;
  };

  // Aliases
  static const uint32_t NriConstant(BGRA_UNUSED) = 0; // only for "bgra" color for profiling
  static const uint32_t NriConstant(ALL) = 0; // only for "sampleMask"
  static const Nri (Dim_t) NriConstant(WHOLE_SIZE) = 0; // only for "Dim_t" and "size"
  static const Nri (Dim_t) NriConstant(REMAINING) = 0; // only for "mipNum" and "layerNum"

  // Readability
#define NriOptional // i.e. can be 0 (keep an eye on comments)
#define NriOut      // highlights an output argument

  // Implicit memory heaps for "CreatePlacedX"
#define NriDeviceHeap 0, 0
#define NriDeviceUploadHeap 0, 1
#define NriHostUploadHeap 0, 2
#define NriHostReadbackHeap 0, 3

  //============================================================================================================================================================================================
#pragma region [ Common ]
  //============================================================================================================================================================================================

  // "AdapterDesc::supportedGraphicsAPIs" is a mask of supported graphics APIs
  ENGINE_BITS(GraphicsBackend, uint8_t,
    None = ENGINE_BIT(0),
    // Supports everything, does nothing, returns dummy non-NULL objects and ~0-filled descs, available if "NRI_ENABLE_NONE_SUPPORT = ON" in CMake
    D3D11 = ENGINE_BIT(1),
    // Direct3D 11 (feature set 11.1), available if "NRI_ENABLE_D3D11_SUPPORT = ON" in CMake (https://microsoft.github.io/DirectX-Specs/d3d/archive/D3D11_3_FunctionalSpec.htm)
    D3D12 = ENGINE_BIT(2),
    // Direct3D 12 (D3D12_SDK_VERSION 4 or 619+), available if "NRI_ENABLE_D3D12_SUPPORT = ON" in CMake (https://microsoft.github.io/DirectX-Specs/)
    Vulkan = ENGINE_BIT(3),
    // Vulkan 1.4+, 1.3++ or 1.2+++ (can be used on MacOS via MoltenVK), available if "NRI_ENABLE_VK_SUPPORT = ON" in CMake (https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html)
    WGPU = ENGINE_BIT(4)
    // WebGPU via "wgpu-native", available if "NRI_ENABLE_WGPU_SUPPORT = ON" in CMake (https://github.com/gfx-rs/wgpu-native). Has limitations similar to D3D11
  );

  enum class Result: int8_t {
    // All bad, but optionally require an action ("callbackInterface.AbortExecution" is not triggered)
    DeviceLost = -3, // may be returned by "QueueSubmit*", "*WaitIdle", "AcquireNextTexture", "QueuePresent", "WaitForPresent"
    OutOfDate = -2, // VK: swap chain is out of date, can be triggered if "features.resizableSwapChain" is not supported; D3D12: shader cache is stale
    InvalidSDK = -1, // D3D12: some interfaces are missing (potential reasons: unable to load "D3D12Core.dll", version or SDK mismatch, developer mode is not enabled)

    // All good
    Success = 0,

    // All bad, most likely a crash or a validation error will happen next ("callbackInterface.AbortExecution" is triggered)
    Failure = 1,
    InvalidArgument = 2,
    OutOfMemory = 3,
    Unsupported = 4 // if enabled, NRI validation can promote some to "INVALID_ARGUMENT"
  };

  // The viewport origin is top-left (D3D native) by default, but can be changed to bottom-left (VK native)
  // https://docs.vulkan.org/refpages/latest/refpages/source/VkViewport.html
  // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_viewport
  struct Viewport {
    float x;
    float y;
    float width;
    float height;
    float depthMin;
    float depthMax;
    bool originBottomLeft; // expects "features.viewportOriginBottomLeft"
  };

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkRect2D.html
  struct Rect {
    int16_t x;
    int16_t y;
    Dim_t width;
    Dim_t height;
  };

  struct Color32f {
    float x, y, z, w;
  };

  struct Color32ui {
    uint32_t x, y, z, w;
  };

  struct Color32i {
    int32_t x, y, z, w;
  };

  struct DepthStencil {
    float depth;
    uint8_t stencil;
  };

  union Color {
    Color32f f;
    Color32ui ui;
    Color32i i;
  };

  struct ClearValue {
    DepthStencil depthStencil;
    Color color;
  };

  struct SampleLocation {
    int8_t x, y; // [-8; 7]
  };

  struct BufferOffset {
    Buffer* buffer;
    uint64_t offset;
  };

  struct DataSize {
    const void *data;
    uint64_t size;
  };

#pragma endregion

  //============================================================================================================================================================================================
#pragma region [ Formats ]
  //============================================================================================================================================================================================
  // https://docs.vulkan.org/refpages/latest/refpages/source/VkFormat.html
  // https://learn.microsoft.com/en-us/windows/win32/api/dxgiformat/ne-dxgiformat-dxgi_format
  // left -> right : low -> high bits
  // Expected (but not guaranteed) "FormatSupportBits" are provided, but "GetFormatSupport" should be used for querying real HW support
  // To demote sRGB use the previous format, i.e. "format - 1"
  //                                            STORAGE_WRITE_WITHOUT_FORMAT
  //                                           STORAGE_READ_WITHOUT_FORMAT |
  //                                                       VERTEX_BUFFER | |
  //                                            STORAGE_BUFFER_ATOMICS | | |
  //                                                  STORAGE_BUFFER | | | |
  //                                                        BUFFER | | | | |
  //                                         MULTISAMPLE_RESOLVE | | | | | |
  //                                            MULTISAMPLE_8X | | | | | | |
  //                                          MULTISAMPLE_4X | | | | | | | |
  //                                        MULTISAMPLE_2X | | | | | | | | |
  //                                               BLEND | | | | | | | | | |
  //                          DEPTH_STENCIL_ATTACHMENT | | | | | | | | | | |
  //                                COLOR_ATTACHMENT | | | | | | | | | | | |
  //                       STORAGE_TEXTURE_ATOMICS | | | | | | | | | | | | |
  //                             STORAGE_TEXTURE | | | | | | | | | | | | | |
  //                                   TEXTURE | | | | | | | | | | | | | | |
  //                                         | | | | | | | | | | | | | | | |
  enum class DataFormat: uint8_t {
    // |      FormatSupportBits      |
    Unknown,                            // . . . . . . . . . . . . . . . .

    // Plain: 8 bits per channel
    R8_UNORM,                           // + + . + . + + + + + + + . + + +
    R8_SNORM,                           // + + . + . + + + + + + + . + + +
    R8_UINT,                            // + + . + . . + + + . + + . + + +  // SHADING_RATE compatible, see NRI_SHADING_RATE macro
    R8_SINT,                            // + + . + . . + + + . + + . + + +

    RG8_UNORM,                          // + + . + . + + + + + + + . + + +  // "AccelerationStructure" compatible (requires "tiers.rayTracing >= 2")
    RG8_SNORM,                          // + + . + . + + + + + + + . + + +  // "AccelerationStructure" compatible (requires "tiers.rayTracing >= 2")
    RG8_UINT,                           // + + . + . . + + + . + + . + + +
    RG8_SINT,                           // + + . + . . + + + . + + . + + +

    BGRA8_UNORM,                        // + + . + . + + + + + + + . + + +
    BGRA8_SRGB,                         // + . . + . + + + + + . . . . . .

    RGBA8_UNORM,                        // + + . + . + + + + + + + . + + +  // "AccelerationStructure" compatible (requires "tiers.rayTracing >= 2")
    RGBA8_SRGB,                         // + . . + . + + + + + . . . . . .
    RGBA8_SNORM,                        // + + . + . + + + + + + + . + + +  // "AccelerationStructure" compatible (requires "tiers.rayTracing >= 2")
    RGBA8_UINT,                         // + + . + . . + + + . + + . + + +
    RGBA8_SINT,                         // + + . + . . + + + . + + . + + +

    // Plain: 16 bits per channel
    R16_UNORM,                          // + + . + . + + + + + + + . + + +
    R16_SNORM,                          // + + . + . + + + + + + + . + + +
    R16_UINT,                           // + + . + . . + + + . + + . + + +
    R16_SINT,                           // + + . + . . + + + . + + . + + +
    R16_SFLOAT,                         // + + . + . + + + + + + + . + + +

    RG16_UNORM,                         // + + . + . + + + + + + + . + + +  // "AccelerationStructure" compatible (requires "tiers.rayTracing >= 2")
    RG16_SNORM,                         // + + . + . + + + + + + + . + + +  // "AccelerationStructure" compatible
    RG16_UINT,                          // + + . + . . + + + . + + . + + +
    RG16_SINT,                          // + + . + . . + + + . + + . + + +
    RG16_SFLOAT,                        // + + . + . + + + + + + + . + + +  // "AccelerationStructure" compatible

    RGBA16_UNORM,                       // + + . + . + + + + + + + . + + +  // "AccelerationStructure" compatible (requires "tiers.rayTracing >= 2")
    RGBA16_SNORM,                       // + + . + . + + + + + + + . + + +  // "AccelerationStructure" compatible
    RGBA16_UINT,                        // + + . + . . + + + . + + . + + +
    RGBA16_SINT,                        // + + . + . . + + + . + + . + + +
    RGBA16_SFLOAT,                      // + + . + . + + + + + + + . + + +  // "AccelerationStructure" compatible

    // Plain: 32 bits per channel
    R32_UINT,                           // + + + + . . + + + . + + + + + +
    R32_SINT,                           // + + + + . . + + + . + + + + + +
    R32_SFLOAT,                         // + + + + . + + + + + + + + + + +

    RG32_UINT,                          // + + . + . . + + + . + + . + + +
    RG32_SINT,                          // + + . + . . + + + . + + . + + +
    RG32_SFLOAT,                        // + + . + . + + + + + + + . + + +  // "AccelerationStructure" compatible

    RGB32_UINT,                         // + . . . . . . . . . + . . + . .
    RGB32_SINT,                         // + . . . . . . . . . + . . + . .
    RGB32_SFLOAT,                       // + . . . . . . . . + + . . + . .  // "AccelerationStructure" compatible

    RGBA32_UINT,                        // + + . + . . + + + . + + . + + +
    RGBA32_SINT,                        // + + . + . . + + + . + + . + + +
    RGBA32_SFLOAT,                      // + + . + . + + + + + + + . + + +

    // Packed: 16 bits per pixel
    B5_G6_R5_UNORM,                     // + . . + . + + + + + . . . . . .
    B5_G5_R5_A1_UNORM,                  // + . . + . + + + + + . . . . . .
    B4_G4_R4_A4_UNORM,                  // + . . . . . . . . + . . . . . .

    // Packed: 32 bits per pixel
    R10_G10_B10_A2_UNORM,               // + + . + . + + + + + + + . + + +  // "AccelerationStructure" compatible (requires "tiers.rayTracing >= 2")
    R10_G10_B10_A2_UINT,                // + + . + . . + + + . + + . + + +
    R11_G11_B10_UFLOAT,                 // + + . + . + + + + + + + . + + +
    R9_G9_B9_E5_UFLOAT,                 // + . . . . . . . . . . . . . . .

    // Block-compressed (requires "features.textureCompressionBC")
    // https://learn.microsoft.com/en-us/windows/win32/direct3d11/texture-block-compression-in-direct3d-11?source=recommendations
    // https://registry.khronos.org/DataFormat/specs/1.4/dataformat.1.4.html#S3TC
    // https://registry.khronos.org/DataFormat/specs/1.4/dataformat.1.4.html#RGTC
    // https://registry.khronos.org/DataFormat/specs/1.4/dataformat.1.4.html#BPTC
    BC1_RGBA_UNORM,                     // + . . . . . . . . . . . . . . .
    BC1_RGBA_SRGB,                      // + . . . . . . . . . . . . . . .
    BC2_RGBA_UNORM,                     // + . . . . . . . . . . . . . . .
    BC2_RGBA_SRGB,                      // + . . . . . . . . . . . . . . .
    BC3_RGBA_UNORM,                     // + . . . . . . . . . . . . . . .
    BC3_RGBA_SRGB,                      // + . . . . . . . . . . . . . . .
    BC4_R_UNORM,                        // + . . . . . . . . . . . . . . .
    BC4_R_SNORM,                        // + . . . . . . . . . . . . . . .
    BC5_RG_UNORM,                       // + . . . . . . . . . . . . . . .
    BC5_RG_SNORM,                       // + . . . . . . . . . . . . . . .
    BC6H_RGB_UFLOAT,                    // + . . . . . . . . . . . . . . .
    BC6H_RGB_SFLOAT,                    // + . . . . . . . . . . . . . . .
    BC7_RGBA_UNORM,                     // + . . . . . . . . . . . . . . .
    BC7_RGBA_SRGB,                      // + . . . . . . . . . . . . . . .

    // Block-compressed: Ericsson Texture Compression (requires "features.textureCompressionETC2")
    // https://registry.khronos.org/DataFormat/specs/1.4/dataformat.1.4.html#ETC2
    ETC2_RGB8_UNORM,                    // + . . . . . . . . . . . . . . .
    ETC2_RGB8_SRGB,                     // + . . . . . . . . . . . . . . .
    ETC2_RGB8_A1_UNORM,                 // + . . . . . . . . . . . . . . .
    ETC2_RGB8_A1_SRGB,                  // + . . . . . . . . . . . . . . .
    ETC2_RGB8_A8_UNORM,                 // + . . . . . . . . . . . . . . .
    ETC2_RGB8_A8_SRGB,                  // + . . . . . . . . . . . . . . .
    ETC2_R11_UNORM,                     // + . . . . . . . . . . . . . . .
    ETC2_R11_SNORM,                     // + . . . . . . . . . . . . . . .
    ETC2_R11_G11_UNORM,                 // + . . . . . . . . . . . . . . .
    ETC2_R11_G11_SNORM,                 // + . . . . . . . . . . . . . . .

    // Block-compressed: Adaptive Scalable Texture Compression (requires "features.textureCompressionASTC")
    // https://registry.khronos.org/DataFormat/specs/1.4/dataformat.1.4.html#ASTC
    ASTC_4X4_UNORM,                     // + . . . . . . . . . . . . . . .
    ASTC_4X4_SRGB,                      // + . . . . . . . . . . . . . . .
    ASTC_5X4_UNORM,                     // + . . . . . . . . . . . . . . .
    ASTC_5X4_SRGB,                      // + . . . . . . . . . . . . . . .
    ASTC_5X5_UNORM,                     // + . . . . . . . . . . . . . . .
    ASTC_5X5_SRGB,                      // + . . . . . . . . . . . . . . .
    ASTC_6X5_UNORM,                     // + . . . . . . . . . . . . . . .
    ASTC_6X5_SRGB,                      // + . . . . . . . . . . . . . . .
    ASTC_6X6_UNORM,                     // + . . . . . . . . . . . . . . .
    ASTC_6X6_SRGB,                      // + . . . . . . . . . . . . . . .
    ASTC_8X5_UNORM,                     // + . . . . . . . . . . . . . . .
    ASTC_8X5_SRGB,                      // + . . . . . . . . . . . . . . .
    ASTC_8X6_UNORM,                     // + . . . . . . . . . . . . . . .
    ASTC_8X6_SRGB,                      // + . . . . . . . . . . . . . . .
    ASTC_8X8_UNORM,                     // + . . . . . . . . . . . . . . .
    ASTC_8X8_SRGB,                      // + . . . . . . . . . . . . . . .
    ASTC_10X5_UNORM,                    // + . . . . . . . . . . . . . . .
    ASTC_10X5_SRGB,                     // + . . . . . . . . . . . . . . .
    ASTC_10X6_UNORM,                    // + . . . . . . . . . . . . . . .
    ASTC_10X6_SRGB,                     // + . . . . . . . . . . . . . . .
    ASTC_10X8_UNORM,                    // + . . . . . . . . . . . . . . .
    ASTC_10X8_SRGB,                     // + . . . . . . . . . . . . . . .
    ASTC_10X10_UNORM,                   // + . . . . . . . . . . . . . . .
    ASTC_10X10_SRGB,                    // + . . . . . . . . . . . . . . .
    ASTC_12X10_UNORM,                   // + . . . . . . . . . . . . . . .
    ASTC_12X10_SRGB,                    // + . . . . . . . . . . . . . . .
    ASTC_12X12_UNORM,                   // + . . . . . . . . . . . . . . .
    ASTC_12X12_SRGB,                    // + . . . . . . . . . . . . . . .

    // Depth
    D16_UNORM,                          // + . . . + . + + + . . . . . . .
    D32_SFLOAT,                         // + . . . + . + + + . . . . . . .

    // Depth-stencil
    D24_UNORM_S8_UINT,                  // + . . . + . + + + . . . . . . .
    D32_SFLOAT_S8_UINT,                 // + . . . + . + + + . . . . . . .
    Count                               // DataFormat count
  };

  // https://learn.microsoft.com/en-us/windows/win32/direct3d12/subresources#plane-slice
  // https://docs.vulkan.org/refpages/latest/refpages/source/VkImageAspectFlagBits.html
  ENGINE_BITS(PlaneBits, uint8_t,
      All = 0, // lazy default
      None = ENGINE_BIT(7), // no accessible planes (needed for a read-only depth-stencil attachment)

      Color = ENGINE_BIT(0), // indicates "color" plane (same as "ALL" for color formats)

      // D3D11: can't be addressed individually in "copy" and "resolve" operations
      Depth = ENGINE_BIT(1), // indicates "depth" plane (same as "ALL" for depth-only formats)
      Stencil = ENGINE_BIT(2), // indicates "stencil" plane in depth-stencil formats

      // For multi-planar YUV formats
      Plane0 = ENGINE_BIT(3),
      Plane1 = ENGINE_BIT(4),
      Plane2 = ENGINE_BIT(5),

      // Aliases
      PlaneY = Plane0,
      PlaneUV = Plane1
  );

  // A bit represents a feature, supported by a format
  // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_feature_data_format_support
  // https://docs.vulkan.org/refpages/latest/refpages/source/VkFormatFeatureFlagBits2.html
  // WGPU: typed buffer views are unsupported; storage textures cannot be multisampled
  ENGINE_BITS(FormatSupportBits, uint32_t,
    Unsupported = 0, // format is unsupported

    // Texture
    Texture = ENGINE_BIT(0), // sampled texture view
    StorageTexture = ENGINE_BIT(1), // storage texture view
    StorageTextureAtomics = ENGINE_BIT(2), // storage texture atomics other than Load / Store
    ColorAttachment = ENGINE_BIT(3), // color attachment view
    DepthStencilAttachment = ENGINE_BIT(4), // depth-stencil attachment view
    Blend = ENGINE_BIT(5), // color attachment blending
    Multisample2x = ENGINE_BIT(6), // 2x multisampled texture
    Multisample4x = ENGINE_BIT(7), // 4x multisampled texture
    Multisample8x = ENGINE_BIT(8), // 8x multisampled texture
    MultisampleResolve = ENGINE_BIT(9), // resolve source/destination

    // Buffer
    Buffer = ENGINE_BIT(10), // typed buffer view
    StorageBuffer = ENGINE_BIT(11), // typed storage buffer view
    StorageBufferAtomics = ENGINE_BIT(12), // typed storage buffer atomics other than Load / Store
    VertexBuffer = ENGINE_BIT(13), // vertex buffer attribute

    // Texture / buffer
    StorageReadWithoutFormat = ENGINE_BIT(14), // storage read with unknown format
    StorageWriteWithoutFormat = ENGINE_BIT(15), // storage write with unknown format

    // Host (generally supported for non-depth/stencil formats with "TEXTURE" bit support)
    HostCopy = ENGINE_BIT(16) // synchronous host copies are supported
  );

#pragma endregion

  //============================================================================================================================================================================================
#pragma region [ Pipeline stages and barriers ]
  //============================================================================================================================================================================================

  // https://microsoft.github.io/DirectX-Specs/d3d/D3D12EnhancedBarriers.html
  // https://docs.vulkan.org/samples/latest/samples/performance/pipeline_barriers/README.html

  // A barrier consists of two phases:
  // - before (source scope, 1st synchronization scope):
  //   - "AccessBits" corresponding with any relevant resource usage since the preceding barrier or the start of "QueueSubmit" scope
  //   - "StagesBits" of all preceding GPU work that must be completed before executing the barrier (stages to wait before the barrier)
  //   - "Layout" for textures
  // - after (destination scope, 2nd synchronization scope):
  //   - "AccessBits" corresponding with any relevant resource usage after the barrier completes
  //   - "StagesBits" of all subsequent GPU work that must wait until the barrier execution is finished (stages to halt until the barrier is executed)
  //   - "Layout" for textures
  // If "features.enhancedBarriers" is not supported:
  //   - https://microsoft.github.io/DirectX-Specs/d3d/D3D12EnhancedBarriers.html#compatibility-with-legacy-d3d12_resource_states
  //   - "AccessBits::NONE" gets mapped to "COMMON" (aka "GENERAL" access), leading to potential discrepancies with VK

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkPipelineStageFlagBits2.html
  // https://microsoft.github.io/DirectX-Specs/d3d/D3D12EnhancedBarriers.html#d3d12_barrier_sync
  ENGINE_BITS(StageBits, uint32_t,
    // Special
    All = 0, // Lazy default for barriers                          Shader stage
    None = 0x7FFFFFFF,

    // Graphics                                     // Invoked by "CmdDraw*"
    IndexInput = ENGINE_BIT(0), //    Index buffer consumption
    VertexShader = ENGINE_BIT(1),
    //    Vertex shader                                   X (required within GRAPHICS bind point)
    TessellationControlShader = ENGINE_BIT(2), //    Tessellation control (hull) shader              X
    TessellationEvaluationShader = ENGINE_BIT(3), //    Tessellation evaluation (domain) shader         X
    GeometryShader = ENGINE_BIT(4), //    Geometry shader                                 X
    TaskShader = ENGINE_BIT(5), //    Task (amplification) shader                     X
    MeshShader = ENGINE_BIT(6),
    //    Mesh shader                                     X (or required within GRAPHICS bind point)
    FragmentShader = ENGINE_BIT(7), //    Fragment (pixel) shader                         X
    DepthStencilAttachment = ENGINE_BIT(8), //    Depth-stencil R/W operations
    ColorAttachment = ENGINE_BIT(9), //    Color R/W operations
    ShadingRateAttachment = ENGINE_BIT(10), //    Shading rate attachment R

    // Compute                                      // Invoked by "CmdDispatch*" (not Rays)
    ComputeShader = ENGINE_BIT(11),
    //    Compute shader                                  X (required within COMPUTE bind point)

    // Ray tracing                                  // Invoked by "CmdDispatchRays*"
    RayGenShader = ENGINE_BIT(12),
    //    Ray generation shader                           X (required within RAY_TRACING bind point)
    MissShader = ENGINE_BIT(13), //    Miss shader                                     X
    IntersectionShader = ENGINE_BIT(14), //    Intersection shader                             X
    ClosestHitShader = ENGINE_BIT(15), //    Closest hit shader                              X
    AnyHitShader = ENGINE_BIT(16), //    Any hit shader                                  X
    CallableShader = ENGINE_BIT(17), //    Callable shader                                 X
    AccelerationStructure = ENGINE_BIT(18), // Invoked by "Cmd*AccelerationStructure*" commands
    Micromap = ENGINE_BIT(19), // Invoked by "Cmd*Micromap*" commands

    // Other
    Copy = ENGINE_BIT(20), // Invoked by "CmdCopy*", "CmdUpload*" and "CmdReadback*"
    Resolve = ENGINE_BIT(21), // Invoked by "CmdResolveTexture"
    ClearStorage = ENGINE_BIT(22), // Invoked by "CmdClearStorage"

    // Modifiers
    Indirect = ENGINE_BIT(23), // Invoked by "Indirect" commands (used in addition to other bits)

    // Host
    Host = ENGINE_BIT(24), // Invoked by "UploadHostMemoryToTexture" and "ReadbackTextureToHostMemory"

    // Video
    VideoDecode = ENGINE_BIT(25), // Invoked by "CmdDecodeVideo"
    VideoEncode = ENGINE_BIT(26), // Invoked by "CmdEncodeVideo"

    // Umbrella stages
    TessellationShaders             = TessellationControlShader | TessellationEvaluationShader,

    MeshShaders                     = TaskShader | MeshShader,

    GraphicsShaders                 = VertexShader | TessellationShaders | GeometryShader | MeshShaders | FragmentShader,

    RayTracingShaders               = RayGenShader | MissShader | IntersectionShader | ClosestHitShader | AnyHitShader | CallableShader,

    AllShaders                      = GraphicsShaders | ComputeShader | RayTracingShaders,

    Graphics                        = IndexInput | GraphicsShaders | DepthStencilAttachment | ColorAttachment | ShadingRateAttachment
  );

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkAccessFlagBits2.html
  // https://microsoft.github.io/DirectX-Specs/d3d/D3D12EnhancedBarriers.html#d3d12_barrier_access
  ENGINE_BITS(AccessBits, uint32_t,
    None = 0,
    // Mapped to "COMMON" (aka "GENERAL" access), if AgilitySDK is not available, leading to potential discrepancies with VK

    // Buffer                                   // Access   Compatible "StageBits" (including ALL)
    IndexBuffer = ENGINE_BIT(0), // R        INDEX_INPUT
    VertexBuffer = ENGINE_BIT(1), // R        VERTEX_SHADER
    ConstantBuffer = ENGINE_BIT(2), // R        ALL_SHADERS
    ArgumentBuffer = ENGINE_BIT(3), // R        INDIRECT
    ScratchBuffer = ENGINE_BIT(4), // RW       ACCELERATION_STRUCTURE, MICROMAP

    // Attachment
    ColorAttachmentRead = ENGINE_BIT(5), // R        COLOR_ATTACHMENT (implicitly by ROP)
    ColorAttachmentWrite = ENGINE_BIT(6), //  W       COLOR_ATTACHMENT
    DepthStencilAttachmentRead = ENGINE_BIT(7), // R        DEPTH_STENCIL_ATTACHMENT
    DepthStencilAttachmentWrite = ENGINE_BIT(8), //  W       DEPTH_STENCIL_ATTACHMENT
    ShadingRateAttachment = ENGINE_BIT(9), // R        SHADING_RATE_ATTACHMENT
    InputAttachment = ENGINE_BIT(10), // R        FRAGMENT_SHADER

    // Acceleration structure
    AccelerationStructureRead = ENGINE_BIT(11),
    // R        COMPUTE_SHADER, RAY_TRACING_SHADERS, ACCELERATION_STRUCTURE
    AccelerationStructureWrite = ENGINE_BIT(12), //  W       ACCELERATION_STRUCTURE

    // Micromap
    MicromapRead = ENGINE_BIT(13), // R        MICROMAP, ACCELERATION_STRUCTURE
    MicroMapWrite = ENGINE_BIT(14), //  W       MICROMAP

    // Shader
    ShaderResource = ENGINE_BIT(15), // R        ALL_SHADERS
    ShaderResourceStorage = ENGINE_BIT(16), // RW       ALL_SHADERS, CLEAR_STORAGE
    ShaderBindingTable = ENGINE_BIT(17), // R        RAY_TRACING_SHADERS

    // Copy
    CopySource = ENGINE_BIT(18), // R        COPY
    CopyDestination = ENGINE_BIT(19), //  W       COPY

    // Resolve
    ResolveSource = ENGINE_BIT(20), // R        RESOLVE
    ResolveDestination = ENGINE_BIT(21), //  W       RESOLVE

    // Clear storage
    ClearStorage = ENGINE_BIT(22), //  W       CLEAR_STORAGE

    // Host
    HostRead = ENGINE_BIT(23), // R        HOST
    HostWrite = ENGINE_BIT(24), //  W       HOST

    // Video
    VideoDecodeRead = ENGINE_BIT(25), // R        VIDEO_DECODE
    VideoDecodeWrite = ENGINE_BIT(26), //  W       VIDEO_DECODE
    VideoEncodeRead = ENGINE_BIT(27), // R        VIDEO_ENCODE
    VideoEncodeWrite = ENGINE_BIT(28), //  W       VIDEO_ENCODE

    // Umbrella access
    ColorAttachment = ColorAttachmentRead | ColorAttachmentWrite,

    DepthStencilAttachment = DepthStencilAttachmentRead | DepthStencilAttachmentWrite,

    AccelerationStructure = AccelerationStructureRead | AccelerationStructureWrite,

    Micromap = MicromapRead | MicroMapWrite,

    VideoDecode = VideoDecodeRead | VideoDecodeWrite,

    VideoEncode = VideoEncodeRead | VideoEncodeWrite
  );

  // "Layout" is ignored if "features.enhancedBarriers" is not supported
  // https://docs.vulkan.org/refpages/latest/refpages/source/VkImageLayout.html
  // https://microsoft.github.io/DirectX-Specs/d3d/D3D12EnhancedBarriers.html#d3d12_barrier_layout
  enum class Layout: uint8_t {
    // Compatible "AccessBits":
    // Special
    Undefined,
    // https://microsoft.github.io/DirectX-Specs/d3d/D3D12EnhancedBarriers.html#d3d12_barrier_layout_undefined
    General,
    // ALL access, required for "SharingMode::SIMULTANEOUS" (but may be suboptimal if "features.unifiedTextureLayouts" is not supported)
    Present, // NONE (use "after.stages = StageBits::NONE")

    // Attachment
    ColorAttachment, // COLOR_ATTACHMENT_READ/WRITE
    DepthStencilAttachment, // DEPTH_STENCIL_ATTACHMENT_READ/WRITE
    DepthReadOnlyStencilAttachment,
    // DEPTH_STENCIL_ATTACHMENT_READ/WRITE (accessible "planes" = "STENCIL"), SHADER_RESOURCE (accessible "planes" = "DEPTH")
    DepthAttachmentStencilReadOnly,
    // DEPTH_STENCIL_ATTACHMENT_READ/WRITE (accessible "planes" = "DEPTH"), SHADER_RESOURCE (accessible "planes" = "STENCIL")
    DepthStencilReadOnly, // DEPTH_STENCIL_ATTACHMENT_READ  (accessible "planes" = "NONE")
    ShadingRateAttachment, // SHADING_RATE_ATTACHMENT
    InputAttachment, // COLOR_ATTACHMENT, INPUT_ATTACHMENT

    // Shader
    ShaderResource, // SHADER_RESOURCE
    ShaderResourceStorage, // SHADER_RESOURCE_STORAGE

    // Copy
    CopySource, // COPY_SOURCE
    CopyDestination, // COPY_DESTINATION

    // Resolve
    ResolveSource, // RESOLVE_SOURCE
    ResolveDestination, // RESOLVE_DESTINATION

    // Video
    VideoDecodeDst, // VIDEO_DECODE_WRITE
    VideoDecodeDpb, // VIDEO_DECODE_READ/WRITE
    VideoEncodeSrc, // VIDEO_ENCODE_READ
    VideoEncodeDpb // VIDEO_ENCODE_READ/WRITE
};

  struct AccessStage {
    AccessBits access;
    StageBits stages;
  };

  struct AccessLayoutStage {
    AccessBits access;
    Layout layout;
    StageBits stages;
  };

  struct GlobalBarrierDesc {
    AccessStage before;
    AccessStage after;
  };

  struct BufferBarrierDesc {
    Buffer *buffer; // use "GetAccelerationStructureBuffer" and "GetMicromapBuffer" for related barriers
    AccessStage before;
    AccessStage after;
  };

  struct TextureBarrierDesc {
    Texture * texture;
    AccessLayoutStage before;
    AccessLayoutStage after;
    Dim_t mipOffset;
    Dim_t mipNum; // can be "REMAINING"
    Dim_t layerOffset;
    Dim_t layerNum; // can be "REMAINING"
    PlaneBits planes;

    // Queue ownership transfer is potentially needed only for "SharingMode::EXCLUSIVE" textures
    // https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#synchronization-queue-transfers
    NriOptional Queue *srcQueue;
    NriOptional Queue *dstQueue;
  };

  // Using "CmdBarrier" inside a rendering pass is allowed, but only for "Layout::INPUT_ATTACHMENT" access transitions
  // D3D12 filters out "transitioning to the same state" barriers if "features.enhancedBarriers" is not supported
  struct BarrierDesc {
    const GlobalBarrierDesc *globals;
    uint32_t globalNum;
    const BufferBarrierDesc *buffers;
    uint32_t bufferNum;
    const TextureBarrierDesc *textures;
    uint32_t textureNum;
  };

#pragma endregion

  //============================================================================================================================================================================================
#pragma region [ Resources: creation ]
  //============================================================================================================================================================================================

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkImageType.html
  // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_resource_dimension
  enum class TextureDimension: uint8_t {
    Texture1D, // WGPU: arrays and mipmaps are unsupported
    Texture2D,
    Texture3D // arrays are unsupported
  };

  // NRI tries to ease your life and avoid using "queue ownership transfers" (see "TextureBarrierDesc").
  // In most of cases "SharingMode" can be ignored. Where is it needed?
  // - VK: use "EXCLUSIVE" for attachments participating into multi-queue activities to preserve DCC (Delta Color Compression) on some HW
  // - D3D12: use "SIMULTANEOUS" to concurrently use a texture as a "SHADER_RESOURCE" (or "SHADER_RESOURCE_STORAGE") and as a "COPY_DESTINATION" for non overlapping texture regions
  // https://docs.vulkan.org/refpages/latest/refpages/source/VkSharingMode.html
  enum class SharingMode: uint8_t {
    Concurrent,
    // VK: lazy default to avoid dealing with "queue ownership transfers", auto-optimized to "EXCLUSIVE" if all queues have the same type
    Exclusive,
    // VK: may be used for attachments to preserve DCC on some HW in the cost of making a "queue ownership transfer"

    // https://microsoft.github.io/DirectX-Specs/d3d/D3D12EnhancedBarriers.html#single-queue-simultaneous-access
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_resource_flags
    Simultaneous
    // D3D12: strengthened variant of "CONCURRENT", allowing simultaneous multiple readers and one writer for a texture (requires "Layout::GENERAL")
  };

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkImageUsageFlagBits.html
  // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_resource_flags
  ENGINE_BITS(TextureUsageBits, uint16_t, // Min compatible access:                   Usage:
    None = 0,
    ShaderResource = ENGINE_BIT(0), // SHADER_RESOURCE                          Read-only shader resource view (SRV)
    ShaderResourceStorage = ENGINE_BIT(1),
    // SHADER_RESOURCE_STORAGE                  Read/write shader resource view (UAV)
    ColorAttachment = ENGINE_BIT(2), // COLOR_ATTACHMENT                         Color attachment (render target)
    DepthStencilAttachment = ENGINE_BIT(3),
    // DEPTH_STENCIL_ATTACHMENT_READ/WRITE      Depth-stencil attachment (depth-stencil target)
    ShadingRateAttachment = ENGINE_BIT(4),
    // SHADING_RATE_ATTACHMENT                  Shading rate attachment (source)
    InputAttachment = ENGINE_BIT(5),
    // INPUT_ATTACHMENT                         Subpass input (read on-chip tile cache)
    HostTransfer = ENGINE_BIT(6),
    // HOST_READ/HOST_WRITE                     Synchronous copy between texture and host memory
    VideoDecode = ENGINE_BIT(7), // VIDEO_DECODE                             Video decode output / DPB picture
    VideoEncode = ENGINE_BIT(8), // VIDEO_ENCODE                             Video encode input / DPB picture
    VideoReferenceOnly = ENGINE_BIT(9)
    // VIDEO_*                                  Video DPB/reference-only allocation
  );

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkBufferUsageFlagBits.html
  ENGINE_BITS(BufferUsageBits, uint16_t, // Min compatible access:                   Usage:
    None = 0,
    ShaderResource = ENGINE_BIT(0), // SHADER_RESOURCE                          Read-only shader resource view (SRV)
    ShaderResourceStorage = ENGINE_BIT(1),
    // SHADER_RESOURCE_STORAGE                  Read/write shader resource view (UAV)
    Vertex = ENGINE_BIT(2), // VERTEX_BUFFER                            Vertex buffer
    Index = ENGINE_BIT(3), // INDEX_BUFFER                             Index buffer
    Constant = ENGINE_BIT(4),
    // CONSTANT_BUFFER                          Constant buffer (D3D11: can't be combined with other usages)
    Argument = ENGINE_BIT(5), // ARGUMENT_BUFFER                          Argument buffer in "Indirect" commands
    Scratch = ENGINE_BIT(6), // SCRATCH_BUFFER                           Scratch buffer in "CmdBuild*" commands
    ShaderBindingTable = ENGINE_BIT(7),
    // SHADER_BINDING_TABLE                     Shader binding table (SBT) in "CmdDispatchRays*" commands
    AccelerationStructureBuildInput = ENGINE_BIT(8),
    // SHADER_RESOURCE                          Read-only input in "CmdBuildAccelerationStructures" command
    AccelerationStructureStorage = ENGINE_BIT(9),
    // ACCELERATION_STRUCTURE_READ/WRITE        (INTERNAL) acceleration structure storage
    MicromapBuildInput = ENGINE_BIT(10),
    // SHADER_RESOURCE                          Read-only input in "CmdBuildMicromaps" command
    MicroMapStorage = ENGINE_BIT(11), // MICROMAP_READ/WRITE                      (INTERNAL) micromap storage
    VideoDecode = ENGINE_BIT(12), // VIDEO_DECODE                             Video decode bitstream input
    VideoEncode = ENGINE_BIT(13) // VIDEO_ENCODE                             Video encode bitstream output
  );

  enum class VideoCodec: uint8_t {
    None,
    H264,
    H265,
    AV1
  };

  struct TextureDesc {
    TextureType type;
    TextureUsageBits usage;
    Format format;
    Dim_t width;
    Dim_t height;
    Dim_t depth;
    Dim_t mipNum;
    Dim_t layerNum;
    Sample_t sampleNum;
    SharingMode sharingMode;
    VideoCodec videoCodec; // VK: required for video textures
    ClearValue optimizedClearValue;
    // D3D12: not needed on desktop, since any HW can track many clear values
  };

  // - VK: buffers are always created with sharing mode "CONCURRENT" to match D3D12 spec
  // - D3D11: "structureStride != 0" locks this buffer to a single "STRUCTURED" layout, unless "byteAddress" is set to "true"
  // - D3D11: "byteAddress = true" allows to create multiple "STRUCTURED" views for a single resource by treating a "STRUCTURED" view as "BYTE_ADDRESS" (spec violation)
  // - WGPU: typed buffer views are unsupported (i.e. "structureStride = 0" and "byteAddress = false")
  struct BufferDesc {
    uint64_t size;
    uint32_t structureStride; // enable "STRUCTURED" views
    BufferUsageBits usage;
    bool byteAddress; // enable "BYTE_ADDRESS" views
  };

#pragma endregion

  //============================================================================================================================================================================================
#pragma region [ Resources: binding to memory ]
  //============================================================================================================================================================================================

  // Contains some encoded implementation specific details
  typedef uint32_t MemoryType;

  // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_heap_type
  enum class MemoryLocation: uint8_t {
    Device,
    DeviceUpload, // soft fallback to "HOST_UPLOAD" if "deviceUploadHeapSize = 0"
    HostUpload,
    HostReadBack
  };

  // Memory requirements for a resource (buffer or texture)
  struct MemoryDesc {
    uint64_t size;
    uint32_t alignment;
    MemoryType type;
    bool mustBeDedicated; // must be put into a dedicated "Memory" object, containing only 1 object with offset = 0
  };

  // A group of non-dedicated "MemoryDesc"s of the SAME "MemoryType" can be merged into a single memory allocation
  struct AllocateMemoryDesc {
    uint64_t size;
    MemoryType type;

    // https://learn.microsoft.com/en-us/windows/win32/direct3d12/residency
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_residency_priority
    // https://docs.vulkan.org/refpages/latest/refpages/source/VkMemoryPriorityAllocateInfoEXT.html
    float priority; // [-1; 1]: low < 0, normal = 0, high > 0

    // Memory allocation goes through "AMD Virtual Memory Allocator"
    //  - most likely a sub-allocation from a larger allocation
    //  - alignment is the maximum of all "memoryDesc.alignment" values for all resources bound to this allocation
    //  - https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
    //  - https://github.com/GPUOpen-LibrariesAndSDKs/D3D12MemoryAllocator
    struct {
      bool enable;
      NriOptional uint32_t alignment; // by default worst-case alignment applied
    } vma;

    // If "false", may reduce alignment requirements
    bool allowMultisampleTextures;
  };

  // Binding resources to a memory (resources can overlap, i.e. alias)
  struct BindBufferMemoryDesc {
    Buffer *buffer;
    Memory *memory;
    uint64_t offset; // in memory
  };

  struct BindTextureMemoryDesc {
    Texture *texture;
    Memory *memory;
    uint64_t offset; // in memory
  };

#pragma endregion

  //============================================================================================================================================================================================
#pragma region [ Resource views and samplers (descriptors) ]
  //============================================================================================================================================================================================

  // https://microsoft.github.io/DirectX-Specs/d3d/ResourceBinding.html#creating-descriptors

  enum class TextureView: uint8_t {
    // Shader resources         // HLSL type                        Compatible "DescriptorType"     Compatible "TextureDimension"
    Texture, // Texture[1D/2D/3D](MS)            TEXTURE                         1D, 2D, 3D
    TextureArray, // Texture[1D/2D](MS)Array          TEXTURE                         1D, 2D
    TextureCube, // TextureCube                      TEXTURE                             2D
    TextureCubeArray, // TextureCubeArray                 TEXTURE                             2D
    StorageTexture, // RWTexture[1D/2D/3D](MS)          STORAGE_TEXTURE                 1D, 2D, 3D
    StorageTextureArray, // RWTexture[1D/2D](MS)Array        STORAGE_TEXTURE                 1D, 2D
    SubpassInput, // SubpassInput(MS) (non-array)     INPUT_ATTACHMENT                    2D

    // Host-only
    ColorAttachment, //                                                                  1D, 2D, 3D
    DepthStencilAttachment, //                                                                  1D, 2D
    ShadingRateAttachment //                                                                      2D
  };

  enum class BufferView: uint8_t {
    // Shader resources         // HLSL type                        Compatible "DescriptorType"
    Buffer, // Buffer                           BUFFER
    StructuredBuffer, // StructuredBuffer                 STRUCTURED_BUFFER
    ByteAddressBuffer, // ByteAddressBuffer                STRUCTURED_BUFFER
    StorageBuffer, // RWBuffer                         STORAGE_BUFFER
    StorageStructuredBuffer, // RWStructuredBuffer               STORAGE_STRUCTURED_BUFFER
    StorageByteAddressBuffer, // RWByteAddressBuffer              STORAGE_STRUCTURED_BUFFER
    ConstantBuffer // ConstantBuffer                   CONSTANT_BUFFER
  };

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkFilter.html
  // https://docs.vulkan.org/refpages/latest/refpages/source/VkSamplerMipmapMode.html
  // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_filter
  enum class Filter: uint8_t {
    Nearest,
    Linear
  };

  // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_filter_reduction_type
  // https://docs.vulkan.org/refpages/latest/refpages/source/VkSamplerReductionMode.html
  enum class FilterOp: uint8_t {
    Average, // a weighted average (sum) of values in the footprint (default)
    Min,
    // a component-wise minimum of values in the footprint with non-zero weights, requires "features.filterOpMinMax"
    Max
    // a component-wise maximum of values in the footprint with non-zero weights, requires "features.filterOpMinMax"
  };

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkSamplerAddressMode.html
  // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_texture_address_mode
  enum class AddressMode: uint8_t {
    Repeat,
    MirroredRepeat,
    ClampToEdge,

    // WGPU: unsupported
    ClampToBorder,
    MirrorClampToEdge
  };

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkCompareOp.html
  // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_comparison_func
  // R - fragment depth, stencil reference or "SampleCmp" reference
  // D - depth or stencil buffer
  enum class CompareOp: uint8_t {
    None, // test is disabled
    Always, // true
    Never, // false
    Equal, // R == D
    NotEqual, // R != D
    Less, // R < D
    LessEqual, // R <= D
    Greater, // R > D
    GreaterEqual // R >= D
  };

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkComponentSwizzle.html
  // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_shader_component_mapping
  enum class ComponentSwizzle: uint8_t {
    Identity, // format-specific default

    // Requires "features.componentSwizzle"
    Zero, // 0
    One, // 1 or 1.0
    R, // .x component (red)
    G, // .y component (green)
    B, // .z component (blue)
    A // .w component (alpha)
  };

  struct ComponentMapping {
    // Only for non-"STORAGE" views
    ComponentSwizzle r;
    ComponentSwizzle g;
    ComponentSwizzle b;
    ComponentSwizzle a;
  };

  struct TextureViewDesc {
    const Texture *texture;
    TextureView type;
    Format format;
    Dim_t mipOffset;
    Dim_t mipNum; // can be "REMAINING"
    Dim_t layerOffset;
    Dim_t layerNum; // can be "REMAINING"
    Dim_t sliceOffset;
    Dim_t sliceNum; // can be "REMAINING"
    PlaneBits planes;
    // accessible planes (missing planes for a "DEPTH_STENCIL_ATTACHMENT" are considered read-only)
    ComponentMapping components;
  };

  struct BufferViewDesc {
    const Buffer *buffer;
    BufferView type;
    uint64_t offset; // expects "memoryAlignment.bufferShaderResourceOffset" for shader resources
    uint64_t size; // can be "WHOLE_SIZE"
    Format format; // needed for typed views, i.e. "BUFFER" and "STORAGE_BUFFER"
    uint32_t structureStride;
    // needed for structured views, i.e. "STRUCTURED_BUFFER" and "STORAGE_STRUCTURED_BUFFER" (= "BufferDesc::structureStride", if not provided)
  };

  struct AddressModes {
    AddressMode u, v, w;
  };

  struct Filters {
    Filter min, mag, mip;
    FilterOp op;
  };

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkSamplerCreateInfo.html
  // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_sampler_desc
  struct SamplerDesc {
    Filters filters;
    uint8_t anisotropy;
    float mipBias;
    float mipMin;
    float mipMax;
    AddressModes addressModes;
    CompareOp compareOp;
    Color borderColor; // used only with "AddressMode::CLAMP_TO_BORDER"
    bool isInteger;
    bool unnormalizedCoordinates; // requires "shaderFeatures.unnormalizedCoordinates"
  };

#pragma endregion

  //============================================================================================================================================================================================
#pragma region [ Pipeline layout and descriptors management ]
  //============================================================================================================================================================================================

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkPipelineBindPoint.html
  enum class BindPoint: uint8_t {
    Inherit, // inherit from the last "CmdSetPipelineLayout" call
    Graphics,
    Compute,
    RayTracing
  };

  ENGINE_BITS(PipelineLayoutBits, uint8_t,
    None = 0,
    IgnoreGlobalSpirvOffsets = ENGINE_BIT(0), // VK: ignore "DeviceCreationDesc::vkBindingOffsets"
    EnableDrawParametersEmulation = ENGINE_BIT(1),
    // D3D12: enable draw parameters emulation, requires "shaderFeatures.drawParameters"
    EnableDrawIndexEmulation = ENGINE_BIT(2),
    // D3D12: enable draw index emulation, requires "shaderFeatures.drawIndex"

    // Direct indexing has two modes:
    // - "descriptor pool":
    //     "MUTABLE" descriptors + "DIRECTLY_INDEXED" flags + up to two ranges in a set describing resource and sampler "virtual heaps" in a descriptor pool
    //     https://github.com/Microsoft/DirectXShaderCompiler/blob/main/docs/SPIR-V.rst#resourcedescriptorheaps--samplerdescriptorheaps
    //     Default VK bindings can be changed via "-fvk-bind-sampler-heap" and "-fvk-bind-resource-heap" DXC options
    // - "descriptor heap":
    //     NRIDescriptorHeap functionality (requires "features.descriptorHeap") + no descriptor sets + at least one "DIRECTLY_INDEXED" flag
    SamplerHeapDirectlyIndexed = ENGINE_BIT(3), // requires "shaderModel >= 66"
    ResourceHeapDirectlyIndexed = ENGINE_BIT(4) // requires "shaderModel >= 66"
  );

  ENGINE_BITS(DescriptorPoolBits, uint8_t,
    None = 0,
    AllowUpdateAfterSet = ENGINE_BIT(0), // allows "DescriptorSetBits::ALLOW_UPDATE_AFTER_SET"
    CopySource = ENGINE_BIT(1)
    // allows allocated descriptor sets to be used as sources in "CopyDescriptorRanges"; such sets can't be bound
  );

  ENGINE_BITS(DescriptorSetBits, uint8_t,
    None = 0,
    AllowUpdateAfterSet = ENGINE_BIT(0) // allows "DescriptorRangeBits::ALLOW_UPDATE_AFTER_SET"
  );

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkDescriptorBindingFlagBits.html
  ENGINE_BITS(DescriptorRangeBits, uint8_t,
    None = 0,

    // Requires "tiers.resourceBinding >= 1"; descriptor validity is additionally restricted by the tier
    PartiallyBound = ENGINE_BIT(0),
    // descriptors in range may not contain valid descriptors at the time the descriptors are consumed (but referenced descriptors must be valid)
    Array = ENGINE_BIT(1), // descriptors in range are organized into an array

    // Requires "tiers.bindless >= 1" and "tiers.resourceBinding >= 2"
    // VK: only one range per set, resolving to the highest binding number after applying "VKBindingOffsets"
    VariableSizedArray = ENGINE_BIT(2),
    // descriptors in range are organized into a variable-sized array, which size is specified via "variableDescriptorNum" argument of "AllocateDescriptorSets" function

    // https://docs.vulkan.org/samples/latest/samples/extensions/descriptor_indexing/README.html#_update_after_bind_streaming_descriptors_concurrently
    // WGPU: true "update after set" is unsupported because bind groups are immutable; "update + rebind" can work, but previously recorded commands can't be patched
    AllowUpdateAfterSet = ENGINE_BIT(3)
    // descriptors in range can be updated after "CmdSetDescriptorSet" but before "QueueSubmit", also works as "DATA_VOLATILE"
  );

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkDescriptorType.html
  enum class DescriptorType: uint8_t {
    // Typed   HLSL reg    Compatible resources
    // Sampler heap
    Sampler, // -        s           sampler

    // Resource heap
    // - a mutable descriptor is a proxy "union" descriptor for all resource descriptor types, i.e. non-sampler
    // - a mutable descriptor can't be created, it can only be allocated from a pool (i.e. used in a "DescriptorRangeDesc")
    // - a mutable descriptor must "mutate" to any resource descriptor via "UpdateDescriptorRanges" or "CopyDescriptorRanges"
    // - a mutable descriptor range may include any non-sampler descriptors, which may be directly indexed in shaders
    Mutable, // -        -           any non-sampler

    // Optimized resources
    Texture, // +        t           TextureView: TEXTURE, TEXTURE_ARRAY, TEXTURE_CUBE, TEXTURE_CUBE_ARRAY
    StorageTexture, // +        u           TextureView: STORAGE_TEXTURE, STORAGE_TEXTURE_ARRAY
    InputAttachment, // +        -           TextureView: SUBPASS_INPUT

    Buffer, // +        t           BufferView: BUFFER
    StorageBuffer, // +        u           BufferView: STORAGE_BUFFER
    ConstantBuffer, // -        b           BufferView: CONSTANT_BUFFER
    StructuredBuffer, // -        t           BufferView: STRUCTURED_BUFFER, BYTE_ADDRESS_BUFFER
    StorageStructuredBuffer,
    // -        u           BufferView: STORAGE_STRUCTURED_BUFFER, STORAGE_BYTE_ADDRESS_BUFFER

    AccelerationStructure // -        t           acceleration structure, requires "features.rayTracing"
  };

  // "DescriptorRange" consists of "Descriptor" entities
  struct DescriptorRangeDesc {
    uint32_t baseRegisterIndex; // "VKBindingOffsets" not applied to "MUTABLE" and "INPUT_ATTACHMENT" to avoid confusion
    uint32_t descriptorNum; // treated as max size if "VARIABLE_SIZED_ARRAY" flag is set
    DescriptorType descriptorType;
    StageBits shaderStages;
    DescriptorRangeBits flags;
  };

  // "DescriptorSet" consists of "DescriptorRange" entities
  struct DescriptorSetDesc {
    uint32_t registerSpace; // must be unique, avoid big gaps
    const DescriptorRangeDesc *ranges;
    uint32_t rangeNum;
    DescriptorSetBits flags;
  };

  // "PipelineLayout" consists of "DescriptorSet" descriptions and root parameters
  struct RootConstantDesc {
    // aka push constants block
    uint32_t registerIndex;
    uint32_t size; // must be non-zero and a multiple of 4
    StageBits shaderStages;
  };

  struct RootDescriptorDesc {
    // aka push descriptor
    uint32_t registerIndex;
    DescriptorType descriptorType; // a non-typed descriptor type
    StageBits shaderStages;
  };

  // https://learn.microsoft.com/en-us/windows/win32/direct3d12/root-signature-limits#static-samplers
  struct RootSamplerDesc {
    // aka static (immutable) sampler
    uint32_t registerIndex;
    SamplerDesc desc;
    StageBits shaderStages;
  };

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkPipelineLayoutCreateInfo.html
  // https://microsoft.github.io/DirectX-Specs/d3d/ResourceBinding.html#root-signature
  // https://microsoft.github.io/DirectX-Specs/d3d/ResourceBinding.html#root-signature-version-11
  /*
  All indices are local in the currently bound pipeline layout. Pipeline layout example:
      RootConstantDesc                #0          // "rootConstantIndex" - an index in "rootConstants" in the currently bound pipeline layout
      ...

      RootDescriptorDesc              #0          // "rootDescriptorIndex" - an index in "rootDescriptors" in the currently bound pipeline layout
      ...

      RootSamplerDesc                 #0
      ...

      Descriptor set                  #0          // "setIndex" - a descriptor set index in the pipeline layout, provided as an argument or bound to the pipeline
          Descriptor range                #0      // "rangeIndex" - a descriptor range index in the descriptor set
              Descriptor num                  N   // "descriptorIndex" and "baseDescriptor" - a descriptor (base) index in the descriptor range, i.e. sub-range start
          ...
      ...
  */
  struct PipelineLayoutDesc {
    uint32_t rootRegisterSpace; // must be unique, avoid big gaps
    const RootConstantDesc *rootConstants;
    uint32_t rootConstantNum;
    const RootDescriptorDesc *rootDescriptors;
    uint32_t rootDescriptorNum;
    const RootSamplerDesc *rootSamplers;
    uint32_t rootSamplerNum;
    const DescriptorSetDesc *descriptorSets;
    uint32_t descriptorSetNum;
    StageBits shaderStages;
    PipelineLayoutBits flags;
  };

  // Descriptor pool
  // https://learn.microsoft.com/en-us/windows/win32/direct3d12/descriptor-heaps
  // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_descriptor_heap_desc
  // https://docs.vulkan.org/refpages/latest/refpages/source/VkDescriptorPoolCreateInfo.html
  struct DescriptorPoolDesc {
    // Maximum number of descriptor sets that can be allocated from this pool
    uint32_t descriptorSetMaxNum;

    // Resource heap
    // - may be directly indexed in shaders via "RESOURCE_HEAP_DIRECTLY_INDEXED" pipeline layout flag
    // - https://docs.vulkan.org/features/latest/features/proposals/VK_EXT_mutable_descriptor_type.html
    uint32_t mutableMaxNum; // number of "MUTABLE" descriptors, requires "features.mutableDescriptorType"

    // Sampler heap
    // - may be directly indexed in shaders via "SAMPLER_HEAP_DIRECTLY_INDEXED" pipeline layout flag
    // - root samplers do not count (not allocated from a descriptor pool)
    uint32_t samplerMaxNum; // number of "SAMPLER" descriptors

    // Optimized resources (may have various sizes depending on Vulkan implementation)
    uint32_t constantBufferMaxNum; // number of "CONSTANT_BUFFER" descriptors
    uint32_t textureMaxNum; // number of "TEXTURE" descriptors
    uint32_t storageTextureMaxNum; // number of "STORAGE_TEXTURE" descriptors
    uint32_t bufferMaxNum; // number of "BUFFER" descriptors
    uint32_t storageBufferMaxNum; // number of "STORAGE_BUFFER" descriptors
    uint32_t structuredBufferMaxNum; // number of "STRUCTURED_BUFFER" descriptors
    uint32_t storageStructuredBufferMaxNum; // number of "STORAGE_STRUCTURED_BUFFER" descriptors
    uint32_t accelerationStructureMaxNum;
    // number of "ACCELERATION_STRUCTURE" descriptors, requires "features.rayTracing"
    uint32_t inputAttachmentMaxNum; // number of "INPUT_ATTACHMENT" descriptors

    DescriptorPoolBits flags;
  };

  // Updating/initializing descriptors in a descriptor set
  struct UpdateDescriptorRangeDesc {
    // Destination
    DescriptorSet *descriptorSet;
    uint32_t rangeIndex;
    uint32_t baseDescriptor;
    // Source & count
    const Descriptor *const *descriptors; // all descriptors must have the same type
    uint32_t descriptorNum;
  };

  // Copying descriptors between descriptor sets
  struct CopyDescriptorRangeDesc {
    // Destination
    DescriptorSet *dstDescriptorSet;
    uint32_t dstRangeIndex;
    uint32_t dstBaseDescriptor;
    // Source & count
    const DescriptorSet *srcDescriptorSet;
    // must be allocated from a "DescriptorPool" with "DescriptorPoolBits::COPY_SOURCE"
    uint32_t srcRangeIndex;
    uint32_t srcBaseDescriptor;
    uint32_t descriptorNum; // must be > 0
  };

  // Binding
  struct SetDescriptorSetDesc {
    uint32_t setIndex; // an index in "PipelineLayoutDesc::descriptorSets"
    const DescriptorSet *descriptorSet;
    BindPoint bindPoint;
  };

  struct SetRootConstantsDesc {
    // requires "pipelineLayout.rootConstantMaxSize > 0", or "descriptorHeap.rootConstantMaxSize > 0" in "descriptor heap" mode
    uint32_t rootConstantIndex; // an index in "PipelineLayoutDesc::rootConstants"
    const void *data;
    uint32_t size;
    uint32_t offset; // requires "features.rootConstantsOffset"
    BindPoint bindPoint;
  };

  struct SetRootDescriptorDesc {
    // requires "pipelineLayout.rootDescriptorMaxNum > 0", or "descriptorHeap.rootDescriptorMaxNum > 0" in "descriptor heap" mode
    uint32_t rootDescriptorIndex; // an index in "PipelineLayoutDesc::rootDescriptors"
    Descriptor *descriptor;
    uint32_t offset; // a non-"CONSTANT_BUFFER" descriptor requires "features.nonConstantBufferRootDescriptorOffset"
    BindPoint bindPoint;
  };

#pragma endregion

  //============================================================================================================================================================================================
#pragma region [ Graphics pipeline: input assembly ]
  //============================================================================================================================================================================================

  enum class IndexType: uint8_t {
    Uint16,
    Uint32
  };

  enum class PrimitiveRestart: uint8_t {
    Disabled,
    IndicesUint16, // index "0xFFFF" enforces primitive restart
    IndicesUint32 // index "0xFFFFFFFF" enforces primitive restart
  };

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkVertexInputRate.html
  enum class VertexStreamStepRate: uint8_t {
    PerVertex,
    PerInstance
  };

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkPrimitiveTopology.html
  // https://learn.microsoft.com/en-us/windows/win32/api/d3dcommon/ne-d3dcommon-d3d_primitive_topology
  // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_primitive_topology_type
  enum class Topology: uint8_t {
    PointList,
    LineList,
    LineStrip,
    TriangleList,
    TriangleStrip,

    // WGPU: unsupported
    LineListWithAdjacency,
    LineStripWithAdjacency,
    TriangleListWithAdjacency,
    TriangleStripWithAdjacency,
    PatchList
  };

  struct InputAssemblyDesc {
    Topology topology;
    uint8_t tessControlPointNum;
    PrimitiveRestart primitiveRestart;
  };

  struct VertexAttributeD3D {
    const char *semanticName;
    uint32_t semanticIndex;
  };

  struct VertexAttributeVK {
    uint32_t location;
  };

  struct VertexAttributeDesc {
    VertexAttributeD3D d3d;
    VertexAttributeVK vk;
    uint32_t offset;
    Format format;
    uint16_t streamIndex;
  };

  struct VertexStreamDesc {
    uint16_t bindingSlot;
    VertexStreamStepRate stepRate;
    uint16_t stride; // fallback if "features.extendedDynamicState" is not supported
  };

  struct VertexInputDesc {
    const VertexAttributeDesc *attributes;
    uint8_t attributeNum;
    const VertexStreamDesc *streams;
    uint8_t streamNum;
  };

  struct VertexBufferDesc {
    const Buffer *buffer;
    uint64_t offset;
    uint32_t stride;
    // requires "features.extendedDynamicState", ignored otherwise, use "VertexStreamDesc::stride" instead
  };

#pragma endregion

  //============================================================================================================================================================================================
#pragma region [ Graphics pipeline: rasterization ]
  //============================================================================================================================================================================================

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkPolygonMode.html
  // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_fill_mode
  enum class FillMode: uint8_t {
    Solid,
    WireFrame
  };

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkCullModeFlagBits.html
  // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_cull_mode
  enum class CullMode: uint8_t {
    None,
    Front,
    Back
  };

  // https://docs.vulkan.org/samples/latest/samples/extensions/fragment_shading_rate_dynamic/README.html
  // https://microsoft.github.io/DirectX-Specs/d3d/VariableRateShading.html
  enum class ShadingRate: uint8_t {
    FragmentSize1x1,
    FragmentSize1x2,
    FragmentSize2x1,
    FragmentSize2x2,

    // Require "features.additionalShadingRates"
    FragmentSize2x4,
    FragmentSize4x2,
    FragmentSize4x4
  };

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkFragmentShadingRateCombinerOpKHR.html
  // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_shading_rate_combiner
  //    "primitiveCombiner"      "attachmentCombiner"
  // A   Pipeline shading rate    Result of Op1
  // B   Primitive shading rate   Attachment shading rate
  enum class ShadingRateCombiner: uint8_t {
    Keep, // A

    // Requires "tiers.shadingRate >= 2"
    Replace, // B
    Min, // min(A, B)
    Max, // max(A, B)

    // Requires "features.sumShadingRateCombiner"
    Sum // (A + B) or (A * B)
  };

  /*
  https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#primsrast-depthbias-computation
  https://learn.microsoft.com/en-us/windows/win32/direct3d11/d3d10-graphics-programming-guide-output-merger-stage-depth-bias
  R - minimum resolvable difference
  S - maximum slope

  bias = constant * R + slopeFactor * S
  if (clamp > 0)
      bias = min(bias, clamp)
  else if (clamp < 0)
      bias = max(bias, clamp)

  enabled if constant != 0 or slope != 0
  */
  struct DepthBiasDesc {
    float constant;
    float clamp;
    float slope;
  };

  struct RasterizationDesc {
    DepthBiasDesc depthBias;
    FillMode fillMode;
    CullMode cullMode;
    bool frontCounterClockwise;
    bool depthClamp;
    bool lineSmoothing; // requires "features.lineSmoothing"
    bool conservativeRaster; // requires "tiers.conservativeRaster != 0"
    bool shadingRate;
    // requires "tiers.shadingRate != 0", expects "CmdSetShadingRate" and optionally "RenderingDesc::shadingRate"
  };

  struct MultisampleDesc {
    uint32_t sampleMask; // can be "ALL"
    Sample_t sampleNum;
    bool alphaToCoverage;
    bool sampleLocations; // requires "tiers.sampleLocations != 0", expects "CmdSetSampleLocations"
  };

  struct ShadingRateDesc {
    ShadingRate shadingRate;
    ShadingRateCombiner primitiveCombiner;
    ShadingRateCombiner attachmentCombiner;
  };

#pragma endregion

  //============================================================================================================================================================================================
#pragma region [ Graphics pipeline: output merger ]
  //============================================================================================================================================================================================

  enum class Multiview: uint8_t {
    // Destination "viewport" and/or "layer" must be set in shaders explicitly, "viewMask" for rendering can be < than the one used for pipeline creation (D3D12 style)
    Flexible, // requires "features.flexibleMultiview"

    // View instances go to statically assigned corresponding attachment layers, "viewMask" for rendering must match the one used for pipeline creation (VK style)
    LayerBased, // requires "features.layerBasedMultiview"

    // View instances go to statically assigned corresponding viewports, "viewMask" for pipeline creation is unused (D3D11 style)
    ViewportBased // requires "features.viewportBasedMultiview"
  };

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkLogicOp.html
  // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_logic_op
  // S - source color 0
  // D - destination color
  enum class LogicOp: uint8_t {
    None,
    Clear,                      // 0
    And,                        // S & D
    AndReverse,                 // S & ~D
    Copy,                       // S
    AndInverted,                // ~S & D
    Xor,                        // S ^ D
    Or,                         // S | D
    Nor,                        // ~(S | D)
    Equivalent,                 // ~(S ^ D)
    Invert,                     // ~D
    OrReverse,                  // S | ~D
    CopyInverted,               // ~S
    OpInverted,                 // ~S | D
    Nand,                       // ~(S & D)
    Set,                         // 1
    Count
  };

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkStencilOp.html
  // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_stencil_op
  // R - reference, set by "CmdSetStencilReference"
  // D - stencil buffer
  enum class StencilOp: uint8_t {
    Keep,                       // D = D
    Zero,                       // D = 0
    Replace,                    // D = R
    IncrementAndClamp,        // D = min(D++, 255)
    DecrementAndClamp,        // D = max(D--, 0)
    Invert,                     // D = ~D
    IncrementAndWrap,         // D++
    DecrementAndWrap,          // D--
    Count
  };

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkBlendFactor.html
  // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_blend
  // S0 - source color 0
  // S1 - source color 1
  // D - destination color
  // C - blend constants, set by "CmdSetBlendConstants"
  enum class BlendFactor: uint8_t {
    // RGB                               ALPHA
    // RGB                               ALPHA
    Zero,                       // 0                                 0
    One,                        // 1                                 1
    SrcColor,                  // S0.r, S0.g, S0.b                  S0.a
    OneMinusSrcColor,        // 1 - S0.r, 1 - S0.g, 1 - S0.b      1 - S0.a
    DstColor,                  // D.r, D.g, D.b                     D.a
    OneMinusDstColor,        // 1 - D.r, 1 - D.g, 1 - D.b         1 - D.a
    SrcAlpha,                  // S0.a                              S0.a
    OneMinusSrcAlpha,        // 1 - S0.a                          1 - S0.a
    DstAlpha,                  // D.a                               D.a
    OneMinusDstAlpha,        // 1 - D.a                           1 - D.a
    ConstantColor,             // C.r, C.g, C.b                     C.a
    OneMinusConstantColor,   // 1 - C.r, 1 - C.g, 1 - C.b         1 - C.a
    ConstantAlpha,             // C.a                               C.a
    OneMinusConstantAlpha,   // 1 - C.a                           1 - C.a
    SrcAlphaSaturate,         // min(S0.a, 1 - D.a)                1
    Src1Color,                 // S1.r, S1.g, S1.b                  S1.a
    OneMinusSrc1Color,       // 1 - S1.r, 1 - S1.g, 1 - S1.b      1 - S1.a
    Src1Alpha,                 // S1.a                              S1.a
    OneMinusSrc1Alpha,        // 1 - S1.a                          1 - S1.a
    Count
  };

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkBlendOp.html
  // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_blend_op
  // S - source color
  // D - destination color
  // Sf - source factor, produced by "BlendFactor"
  // Df - destination factor, produced by "BlendFactor"
  enum class BlendOp: uint8_t {
    Add,                        // S * Sf + D * Df
    Subtract,                   // S * Sf - D * Df
    ReverseSubtract,           // D * Df - S * Sf
    Min,                        // min(S, D)
    Max,                         // max(S, D)
    Count
  };

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkColorComponentFlagBits.html
  ENGINE_BITS(ColorWriteBits, uint8_t,
    None    = 0,
    R       = ENGINE_BIT(0),
    G       = ENGINE_BIT(1),
    B       = ENGINE_BIT(2),
    A       = ENGINE_BIT(3),

    RGB     = R // "wingdi.h" must not be included after
           | G
           | B,

    RGBA    = RGB | A
  );

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkStencilOpState.html
  // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_depth_stencil_desc
  struct StencilDesc {
    CompareOp compareOp; // "compareOp != NONE", expects "CmdSetStencilReference"
    StencilOp failOp;
    StencilOp passOp;
    StencilOp depthFailOp;
    uint8_t writeMask;
    uint8_t compareMask;
  };

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkPipelineDepthStencilStateCreateInfo.html
  struct DepthAttachmentDesc {
    CompareOp compareOp;
    bool write;
    bool boundsTest; // requires "features.depthBoundsTest", expects "CmdSetDepthBounds"
  };

  struct StencilAttachmentDesc {
    StencilDesc front;
    StencilDesc back; // requires "features.independentFrontAndBackStencilReferenceAndMasks" for "back.writeMask"
  };

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkPipelineColorBlendAttachmentState.html
  // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_render_target_blend_desc
  struct BlendDesc {
    BlendFactor srcFactor;
    BlendFactor dstFactor;
    BlendOp op;
  };

  struct ColorAttachmentDesc {
    Format format;
    BlendDesc colorBlend;
    BlendDesc alphaBlend;
    ColorWriteBits colorWriteMask;
    bool blendEnabled;
  };

  struct OutputMergerDesc {
    const ColorAttachmentDesc *colors;
    uint32_t colorNum;
    DepthAttachmentDesc depth;
    StencilAttachmentDesc stencil;
    Format depthStencilFormat;
    LogicOp logicOp; // requires "features.logicOp"
    uint32_t viewMask; // if non-0, requires "viewMaxNum > 1"
    Multiview multiview; // if "viewMask != 0", requires "features.(xxx)Multiview"
  };

#pragma endregion

  //============================================================================================================================================================================================
#pragma region [ Pipelines ]
  //============================================================================================================================================================================================

  // https://docs.vulkan.org/guide/latest/robustness.html
  enum class Robustness: uint8_t {
    Default, // don't care, follow device settings (VK level when used on a device)
      Off, // no overhead, no robust access (out-of-bounds access is not allowed)
      Vulkan, // minimal overhead, partial robust access
      D3D12
      // moderate overhead, D3D12-level robust access (requires "VK_EXT_robustness2", soft fallback to VK mode)
    };

  ENGINE_BITS(GraphicsPipelineBits, uint8_t,
    None                = 0,
    FailOnCacheMiss     = ENGINE_BIT(0) // "CreateGraphicsPipeline" returns "FAILURE" if the pipeline is not found in the supplied cache (requires "features.pipelineCacheControl")
  );

  ENGINE_BITS(ComputePipelineBits, uint8_t,
    None                = 0,
    FailOnCacheMiss     = ENGINE_BIT(0) // "CreateComputePipeline" returns "FAILURE" if the pipeline is not found in the supplied cache (requires "features.pipelineCacheControl")
  );

  struct PipelineCacheDesc {
    const void *data; // "data = NULL" means empty cache
    uint64_t size;
  };

  // It's recommended to use "NRI.hlsl" in the shader code
  struct ShaderInfo {
    StageBits stage;
    const void* bytecode; // see "features.shaderBytecodeXXX"
    uint64_t size;
    const char* entryPointName;
  };

  struct GraphicsPipelineDesc {
    const PipelineLayout *pipelineLayout;
    const VertexInputDesc *vertexInput;
    InputAssemblyDesc inputAssembly;
    RasterizationDesc rasterization;
    const MultisampleDesc *multisample;
    OutputMergerDesc outputMerger;
    const ShaderInfo *shaders;
    uint32_t shaderNum;
    GraphicsPipelineBits flags;
    Robustness robustness;
    const PipelineCache *cache; // uses a cached blob on a hit and stores the result on a miss
  };

  struct ComputePipelineDesc {
    const PipelineLayout *pipelineLayout;
    ShaderDesc shader;
    ComputePipelineBits flags;
    Robustness robustness;
    const PipelineCache *cache; // uses a cached blob on a hit and stores the result on a miss
  };

#pragma endregion

  //============================================================================================================================================================================================
#pragma region [ Rendering (render pass) ]
  //============================================================================================================================================================================================

  // https://docs.vulkan.org/refpages/latest/refpages/source/VK_KHR_dynamic_rendering.html
  // https://github.com/KhronosGroup/Vulkan-Docs/blob/main/proposals/VK_KHR_dynamic_rendering_local_read.adoc

  // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_render_pass_beginning_access_type
  // https://docs.vulkan.org/refpages/latest/refpages/source/VkAttachmentLoadOp.html
  enum class LoadOp : uint8_t {
    Load, // loads the existing attachment contents
    Clear // clears the attachment using "AttachmentDesc::clearValue"
  };

  // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_render_pass_ending_access_type
  // https://docs.vulkan.org/refpages/latest/refpages/source/VkAttachmentStoreOp.html
  enum class StoreOp: uint8_t {
    Store, // stores the attachment contents
    Discard, // makes the attachment contents undefined
    None // performs no store access if the attachment is not written, otherwise acts like "DISCARD"
  };

  // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_resolve_mode
  // https://docs.vulkan.org/refpages/latest/refpages/source/VkResolveModeFlagBits.html
  enum class ResolveOp: uint8_t {
    Average, // resolves the source samples to their average value
    Min, // resolves the source samples to their minimum value, requires "features.resolveOpMinMax"
    Max // resolves the source samples to their maximum value, requires "features.resolveOpMinMax"
  };

  struct AttachmentDesc {
    Descriptor *descriptor;
    ClearValue clearValue;
    LoadOp loadOp;
    StoreOp storeOp;
    ResolveOp resolveOp;
    Descriptor *resolveDst; // must be in "COLOR_ATTACHMENT" state and valid during "CmdEndRendering"
  };

  // If "VK_KHR_dynamic_rendering" is not supported:
  // - "VkRenderPass" is used under the hood
  // - input attachments must be transitioned to "Layout::INPUT_ATTACHMENT" in the same command buffer before "CmdBeginRendering"
  // - matching pipeline input attachment indices are inferred from these transitions
  struct RenderingDesc {
    const AttachmentDesc *colors;
    uint32_t colorNum;
    AttachmentDesc depth; // may be treated as "depth-stencil"
    AttachmentDesc stencil; // (optional) separation is needed for multisample resolve
    const Descriptor *shadingRate; // requires "tiers.shadingRate >= 2"
    uint32_t viewMask; // if non-0, requires "viewMaxNum > 1"
  };

#pragma endregion

  //============================================================================================================================================================================================
#pragma region [ Queries ]
  //============================================================================================================================================================================================

  // https://microsoft.github.io/DirectX-Specs/d3d/CountersAndQueries.html
  // https://docs.vulkan.org/refpages/latest/refpages/source/VkQueryType.html
  enum class QueryType: uint8_t {
    Timestamp,
    // uint64_t, requires "features.timestamp" (for "GRAPHICS" and "COMPUTE" queues), use only "CmdEndQuery" without "CmdBeginQuery"
    TimestampCopyQueue,
    // uint64_t, requires "features.timestampCopyQueue" (for a "COPY" queue), use only "CmdEndQuery" without "CmdBeginQuery"
    Occlusion, // uint64_t, requires "features.occlusion"
    PipelineStatistics, // see "PipelineStatisticsDesc", requires "features.pipelineStatistics"
    AccelerationStructureSize, // uint64_t, requires "features.rayTracing"
    AccelerationStructureCompactedSize, // uint64_t, requires "features.rayTracing"
    MicromapCompactedSize // uint64_t, requires "features.micromap"
  };

  struct QueryPoolDesc {
    QueryType queryType;
    uint32_t capacity;
  };

  // Data layout for QueryType::PIPELINE_STATISTICS
  // https://docs.vulkan.org/refpages/latest/refpages/source/VkQueryPipelineStatisticFlagBits.html
  // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_query_data_pipeline_statistics
  struct PipelineStatisticsDesc {
    // Common part
    uint64_t inputVertexNum;
    uint64_t inputPrimitiveNum;
    uint64_t vertexShaderInvocationNum;
    uint64_t geometryShaderInvocationNum;
    uint64_t geometryShaderPrimitiveNum;
    uint64_t rasterizerInPrimitiveNum;
    uint64_t rasterizerOutPrimitiveNum;
    uint64_t fragmentShaderInvocationNum;
    uint64_t tessControlShaderInvocationNum;
    uint64_t tessEvaluationShaderInvocationNum;
    uint64_t computeShaderInvocationNum;

    // If "features.meshShaderPipelineStats"
    uint64_t taskShaderInvocationNum;
    uint64_t meshShaderInvocationNum;

    // D3D12: if "features.meshShaderPipelineStats"
    uint64_t meshShaderPrimitiveNum;
  };

#pragma endregion

  //============================================================================================================================================================================================
#pragma region [ Command signatures ]
  //============================================================================================================================================================================================

  // To fill commands for indirect drawing in a shader use one of "NRI_FILL_X_DESC" macros

  // Command signatures (default)

  struct DrawDesc {
    // see NRI_FILL_DRAW_DESC
    uint32_t vertexNum;
    uint32_t instanceNum;
    uint32_t baseVertex; // vertex buffer offset = CmdSetVertexBuffers.offset + baseVertex * VertexStreamDesc::stride
    uint32_t baseInstance;
  };

  struct DrawIndexedDesc {
    // see NRI_FILL_DRAW_INDEXED_DESC
    uint32_t indexNum;
    uint32_t instanceNum;
    uint32_t baseIndex;
    // index buffer offset = CmdSetIndexBuffer.offset + baseIndex * sizeof(CmdSetIndexBuffer.indexType)
    int32_t baseVertex; // index += baseVertex
    uint32_t baseInstance;
  };

  struct DispatchDesc {
    uint32_t workGroupNumX;
    uint32_t workGroupNumY;
    uint32_t workGroupNumZ;
  };

  // Modified draw command signatures, if the bound pipeline layout has "PipelineLayoutBits::ENABLE_DRAW_PARAMETERS_EMULATION"
  // "PipelineLayoutBits::ENABLE_DRAW_INDEX_EMULATION" does not change the command layout

  struct DrawBaseDesc {
    // see NRI_FILL_DRAW_DESC
    uint32_t shaderEmulatedBaseVertex; // root constant
    uint32_t shaderEmulatedBaseInstance; // root constant
    uint32_t vertexNum;
    uint32_t instanceNum;
    uint32_t baseVertex; // vertex buffer offset = CmdSetVertexBuffers.offset + baseVertex * VertexStreamDesc::stride
    uint32_t baseInstance;
  };

  struct DrawIndexedBaseDesc {
    // see NRI_FILL_DRAW_INDEXED_DESC
    int32_t shaderEmulatedBaseVertex; // root constant
    uint32_t shaderEmulatedBaseInstance; // root constant
    uint32_t indexNum;
    uint32_t instanceNum;
    uint32_t baseIndex;
    // index buffer offset = CmdSetIndexBuffer.offset + baseIndex * sizeof(CmdSetIndexBuffer.indexType)
    int32_t baseVertex; // index += baseVertex
    uint32_t baseInstance;
  };

#pragma endregion

  //============================================================================================================================================================================================
#pragma region [ Other ]
  //============================================================================================================================================================================================

  // Copy
  struct TextureRegionDesc {
    Dim_t x;
    Dim_t y;
    Dim_t z;
    Dim_t width; // can be "WHOLE_SIZE" (mip)
    Dim_t height; // can be "WHOLE_SIZE" (mip)
    Dim_t depth; // can be "WHOLE_SIZE" (mip)
    Dim_t mipOffset;
    Dim_t layerOffset;
    PlaneBits planes;
  };

  struct TextureDataLayoutDesc {
    uint64_t offset;
    // a buffer offset must be a multiple of "uploadBufferTextureSliceAlignment" (data placement alignment)
    uint32_t rowPitch; // must be a multiple of "uploadBufferTextureRowAlignment"
    uint32_t slicePitch; // must be a multiple of "uploadBufferTextureSliceAlignment"
  };

  struct UploadHostMemoryToTextureDesc {
    const void *srcData;
    Texture *dstTexture; // must be in "{AccessBits::HOST_WRITE, Layout::GENERAL, StageBits::HOST}"
    TextureRegionDesc dstRegion;
    uint32_t srcRowPitch; // if rows are not tightly packed
    uint32_t srcSlicePitch; // if slices are not tightly packed
  };

  struct ReadbackTextureToHostMemoryDesc {
    Texture *srcTexture; // must be in "{AccessBits::HOST_READ, Layout::GENERAL, StageBits::HOST}"
    void *dstData;
    TextureRegionDesc srcRegion;
    uint32_t dstRowPitch; // if rows are not tightly packed
    uint32_t dstSlicePitch; // if slices are not tightly packed
  };

  // Work submission
  struct FenceSubmitDesc {
    Fence *fence;
    uint64_t value;
    StageBits stages;
  };

  struct QueueSubmitDesc {
    const FenceSubmitDesc *waitFences;
    uint32_t waitFenceNum;
    const CommandBuffer *const *commandBuffers;
    uint32_t commandBufferNum;
    const FenceSubmitDesc *signalFences;
    uint32_t signalFenceNum;

    // Required if "NRILowLatency" is enabled for the swap chain
    const SwapChain *swapChain;
    uint64_t presentId; // must match the value passed to "QueuePresent" for the frame
  };

  // Clear
  struct ClearAttachmentDesc {
    ClearValue value;
    PlaneBits planes;
    uint8_t colorAttachmentIndex;
  };

  // Required synchronization
  // - variant 1: "SHADER_RESOURCE_STORAGE" access ("SHADER_RESOURCE_STORAGE" layout) and "CLEAR_STORAGE" stage + any shader stage (or "ALL")
  // - variant 2: "CLEAR_STORAGE" access ("SHADER_RESOURCE_STORAGE" layout) and "CLEAR_STORAGE" stage
  struct ClearStorageDesc {
    // For any buffers and textures with integer formats:
    //  - Clears a storage descriptor with bit-precise values, copying the lower "N" bits from "value.[f/ui/i].channel"
    //    to the corresponding channel, where "N" is the number of bits in the "channel" of the resource format
    // For textures with non-integer formats:
    //  - Clears a storage descriptor with float values with format conversion from "FLOAT" to "UNORM/SNORM" where appropriate
    // For buffers:
    //  - To avoid discrepancies in behavior between GAPIs use "R32f/ui/i" formats for views
    //  - D3D: structured buffers are unsupported!
    Descriptor *descriptor; // a "STORAGE" descriptor
    Color value; // avoid overflow
    uint32_t setIndex;
    uint32_t rangeIndex;
    uint32_t descriptorIndex;
  };

#pragma endregion

  //============================================================================================================================================================================================
#pragma region [ Device description and capabilities ]
  //============================================================================================================================================================================================

  enum class Vendor: uint8_t {
    Unknown,
    Nvidia,
    AMD,
    Intel
  };

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkPhysicalDeviceType.html
  enum class Architecture: uint8_t {
    Unknown,
    Software, // CPU
    Virtual, // remote desktop?
    Integrated, // UMA
    Discrete // yes, please!
  };

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkQueueFlagBits.html
  // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_command_list_type
  enum class QueueType: uint8_t {
    Graphics,
    Compute,
    Copy,
    VideoDecode,
    VideoEncode
  };

  struct AdapterDesc {
    char name[256];
    Uid_t uid; // "LUID" (preferred) if "uid.high = 0", or "UUID" otherwise
    uint64_t videoMemorySize;
    uint64_t sharedSystemMemorySize;
    uint32_t deviceId;
    uint32_t driverVersion; // GAPI and OS dependent
    uint32_t queueNum[(uint32_t)NriScopedMember(QueueType, MAX_NUM)];
    // per type; queues of different types may alias the same native queue
    Vendor vendor;
    Architecture architecture;
    GraphicsBackend supportedGraphicsAPIs;
  };

#define NriShaderModel(major, minor) (major * 100 + minor)

  // Feature support coverage: https://vulkan.gpuinfo.org/ and https://d3d12infodb.boolka.dev/
  struct DeviceInfo {
    // Common
    AdapterDesc adapterDesc; // "queueNum" reflects available number of queues per "QueueType"
    GraphicsBackend graphicsAPI;
    uint16_t nriVersion;
    uint16_t shaderModel; // see "NriShaderModel"

    // Viewport
    struct {
      uint32_t maxNum;
      int32_t boundsMin;
      int32_t boundsMax;
    } viewport;

    // Dimensions
    struct {
      uint32_t typedBufferMaxDim;
      Dim_t attachmentMaxDim;
      Dim_t attachmentLayerMaxNum;
      Dim_t texture1DMaxDim;
      Dim_t texture2DMaxDim;
      Dim_t texture3DMaxDim;
      Dim_t textureLayerMaxNum;
    } dimensions;

    // Precision bits
    struct {
      uint32_t viewportBits;
      uint32_t subPixelBits;
      uint32_t subTexelBits;
      uint32_t mipmapBits;
    } precision;

    // Memory
    struct {
      uint64_t deviceUploadHeapSize; // ReBAR
      uint64_t bufferMaxSize;
      uint64_t allocationMaxSize;
      uint32_t allocationMaxNum;
      uint32_t samplerAllocationMaxNum;
      uint32_t constantBufferMaxRange;
      uint32_t storageBufferMaxRange;
      uint32_t bufferTextureGranularity;
      // specifies a page-like granularity at which linear and non-linear resources must be placed in adjacent memory locations to avoid aliasing
      uint32_t alignmentDefault;
      // (INTERNAL) worst-case alignment for a memory allocation respecting all possible placed resources, excluding multisample textures
      uint32_t alignmentMultisample;
      // (INTERNAL) worst-case alignment for a memory allocation respecting all possible placed resources, including multisample textures
    } memory;

    // Memory alignment requirements
    struct {
      uint32_t uploadBufferTextureRow;
      uint32_t uploadBufferTextureSlice;
      uint32_t bufferShaderResourceOffset;
      uint32_t constantBufferOffset;
      uint32_t scratchBufferOffset;
      uint32_t shaderBindingTable;
      uint32_t accelerationStructureOffset;
      uint32_t micromapOffset;
    } memoryAlignment;

    // Pipeline layout (see "nriFitPipelineLayoutSettingsIntoDeviceLimits")
    // D3D12 only: "rootConstantSize" + "descriptorSetNum" * 4 + "rootDescriptorNum" * 8 + "reservedSize" <= 256, where
    // "reservedSize" is 8 bytes for "ENABLE_DRAW_PARAMETERS_EMULATION" and 4 bytes for "ENABLE_DRAW_INDEX_EMULATION"
    struct {
      uint32_t descriptorSetMaxNum;
      uint32_t rootConstantMaxSize;
      uint32_t rootDescriptorMaxNum;
      uint32_t rootSamplerMaxNum;
    } pipelineLayout;

    // Descriptor set
    struct {
      uint32_t samplerMaxNum;
      uint32_t constantBufferMaxNum;
      uint32_t storageBufferMaxNum;
      uint32_t textureMaxNum;
      uint32_t storageTextureMaxNum;

      struct {
        uint32_t samplerMaxNum;
        uint32_t constantBufferMaxNum;
        uint32_t storageBufferMaxNum;
        uint32_t textureMaxNum;
        uint32_t storageTextureMaxNum;
      } updateAfterSet;
    } descriptorSet;

    // Descriptor heap
    struct {
      uint32_t resourceMaxNum;
      uint32_t samplerMaxNum;
      uint32_t rootConstantMaxSize;
      uint32_t rootDescriptorMaxNum;
      uint32_t rootSamplerMaxNum;
    } descriptorHeap;

    // Shader stages
    struct {
      // Per stage resources
      uint32_t descriptorSamplerMaxNum;
      uint32_t descriptorConstantBufferMaxNum;
      uint32_t descriptorStorageBufferMaxNum;
      uint32_t descriptorTextureMaxNum;
      uint32_t descriptorStorageTextureMaxNum;
      uint32_t resourceMaxNum;

      struct {
        uint32_t descriptorSamplerMaxNum;
        uint32_t descriptorConstantBufferMaxNum;
        uint32_t descriptorStorageBufferMaxNum;
        uint32_t descriptorTextureMaxNum;
        uint32_t descriptorStorageTextureMaxNum;
        uint32_t resourceMaxNum;
      } updateAfterSet;

      // Vertex
      struct {
        uint32_t attributeMaxNum;
        uint32_t streamMaxNum;
        uint32_t outputComponentMaxNum;
      } vertex;

      // Tessellation control
      struct {
        float generationMaxLevel;
        uint32_t patchPointMaxNum;
        uint32_t perVertexInputComponentMaxNum;
        uint32_t perVertexOutputComponentMaxNum;
        uint32_t perPatchOutputComponentMaxNum;
        uint32_t totalOutputComponentMaxNum;
      } tesselationControl;

      // Tessellation evaluation
      struct {
        uint32_t inputComponentMaxNum;
        uint32_t outputComponentMaxNum;
      } tesselationEvaluation;

      // Geometry
      struct {
        uint32_t invocationMaxNum;
        uint32_t inputComponentMaxNum;
        uint32_t outputComponentMaxNum;
        uint32_t outputVertexMaxNum;
        uint32_t totalOutputComponentMaxNum;
      } geometry;

      // Fragment
      struct {
        uint32_t inputComponentMaxNum;
        uint32_t attachmentMaxNum;
        uint32_t dualSourceAttachmentMaxNum;
      } fragment;

      // Compute
      //  - a "dispatch" consists of "work groups" (aka "thread groups")
      //  - a "work group" consists of "waves" (aka "subgroups" or "warps")
      //  - a "wave" consists of "lanes", which can can be active, inactive or a helper:
      //    - active: the "lane" is performing its computations
      //    - inactive: the "lane" is part of the "wave" but is currently masked out
      //    - helper: these "lanes" are executed to provide auxiliary information (like derivatives) for active threads in the same 2x2 quad
      //  - "invocation" (or "thread") is a single shader instance
      //  - "lane" specifically refers to the position of a "thread" within a hardware "wave"
      //  - the concept of "wave/lane" execution applies to all shader stages
      struct {
        uint32_t dispatchMaxDim[3];
        uint32_t workGroupInvocationMaxNum;
        uint32_t workGroupMaxDim[3];
        uint32_t sharedMemoryMaxSize;
      } compute;

      // Task
      struct {
        uint32_t dispatchWorkGroupMaxNum;
        uint32_t dispatchMaxDim[3];
        uint32_t workGroupInvocationMaxNum;
        uint32_t workGroupMaxDim[3];
        uint32_t sharedMemoryMaxSize;
        uint32_t payloadMaxSize;
      } task;

      // Mesh
      struct {
        uint32_t dispatchWorkGroupMaxNum;
        uint32_t dispatchMaxDim[3];
        uint32_t workGroupInvocationMaxNum;
        uint32_t workGroupMaxDim[3];
        uint32_t sharedMemoryMaxSize;
        uint32_t outputVerticesMaxNum;
        uint32_t outputPrimitiveMaxNum;
        uint32_t outputComponentMaxNum;
      } mesh;

      // Ray tracing
      struct {
        uint32_t shaderGroupIdentifierSize;
        uint32_t shaderBindingTableMaxStride;
        uint32_t recursionMaxDepth;
      } rayTracing;
    } shaderStage;

    // Acceleration structure
    struct {
      uint64_t primitiveMaxNum; // per BLAS
      uint64_t geometryMaxNum; // per BLAS
      uint64_t instanceMaxNum; // per TLAS
      uint32_t micromapSubdivisionMaxLevel;
    } accelerationStructure;

    // Wave (subgroup)
    // https://github.com/microsoft/directxshadercompiler/wiki/wave-intrinsics
    // https://microsoft.github.io/DirectX-Specs/d3d/HLSL_SM_6_6_Derivatives.html
    struct {
      uint32_t laneMinNum;
      uint32_t laneMaxNum;
      StageBits waveOpsStages; // SM 6.0+ (see "shaderFeatures.waveX")
      StageBits quadOpsStages; // SM 6.0+ (see "shaderFeatures.waveQuad")
      StageBits derivativeOpsStages;
      // SM 6.6+ (https://microsoft.github.io/DirectX-Specs/d3d/HLSL_SM_6_6_Derivatives.html#derivative-functions)
    } wave;

    // Other
    struct {
      uint64_t timestampFrequencyHz;
      uint32_t drawIndirectMaxNum;
      float samplerLodBiasMax;
      float samplerAnisotropyMax;
      int8_t texelGatherOffsetMin;
      int8_t texelOffsetMin;
      uint8_t texelOffsetMax;
      uint8_t texelGatherOffsetMax;
      uint8_t clipDistanceMaxNum;
      uint8_t cullDistanceMaxNum;
      uint8_t combinedClipAndCullDistanceMaxNum;
      uint8_t viewMaxNum; // multiview is supported if > 1
      uint8_t shadingRateAttachmentTileSize; // square size
      bool timestampCopyQueueResolveOnCopyQueue;
      // if "true", "CmdCopyQueries" for "TIMESTAMP_COPY_QUEUE" requires a COPY queue, otherwise "GRAPHICS" or "COMPUTE"
    } other;

    // Tiers (0 - unsupported)
    struct {
      // https://microsoft.github.io/DirectX-Specs/d3d/ConservativeRasterization.html#tiered-support
      // 1 - 1/2 pixel uncertainty region and does not support post-snap degenerates
      // 2 - reduces the maximum uncertainty region to 1/256 and requires post-snap degenerates not be culled
      // 3 - maintains a maximum 1/256 uncertainty region and adds support for inner input coverage, aka "SV_InnerCoverage"
      uint8_t conservativeRaster;

      // https://microsoft.github.io/DirectX-Specs/d3d/ProgrammableSamplePositions.html#hardware-tiers
      // 1 - a single sample pattern can be specified to repeat for every pixel ("locationNum / sampleNum" ratio must be 1 in "CmdSetSampleLocations"),
      //     1x and 16x sample counts do not support programmable locations
      // 2 - four separate sample patterns can be specified for each pixel in a 2x2 grid ("locationNum / sampleNum" ratio can be 1 or 4 in "CmdSetSampleLocations"),
      //     all sample counts support programmable positions
      uint8_t sampleLocations;

      // https://microsoft.github.io/DirectX-Specs/d3d/Raytracing.html#checkfeaturesupport-structures
      // 1 - DXR 1.0: full raytracing functionality, except features below
      // 2 - DXR 1.1: adds - ray query, "CmdDispatchRaysIndirect", "GeometryIndex()" intrinsic, additional ray flags & vertex formats
      // 3 - DXR 1.2: adds - micromap, shader execution reordering
      uint8_t rayTracing;

      // https://microsoft.github.io/DirectX-Specs/d3d/VariableRateShading.html#feature-tiering
      // 1 - shading rate can be specified only per draw
      // 2 - adds: per primitive shading rate, per "shadingRateAttachmentTileSize" shading rate, combiners, "SV_ShadingRate" support
      uint8_t shadingRate;

      // https://microsoft.github.io/DirectX-Specs/d3d/ResourceBinding.html#limitations-on-static-samplers
      // 0 - ALL descriptors in range must be valid by the time the command list executes
      //       GPUs: rare
      // 1 - only "CONSTANT_BUFFER" and "STORAGE" descriptors in range must be valid
      //       GPUs: NVIDIA GTX 6xx, 7xx, 9xx & 10xx series
      // 2 - only referenced descriptors must be valid
      //       GPUs: NVIDIA GTX 16xx & RTX series, AMD R9 & RX series, Intel Arc & Skylake+
      uint8_t resourceBinding;

      // Descriptor array indexing
      // 1 - unbounded arrays with dynamic indexing
      // 2 - D3D12 dynamic resources: https://microsoft.github.io/DirectX-Specs/d3d/HLSL_SM_6_6_DynamicResources.html
      uint8_t bindless;

      // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_resource_heap_tier
      // 1 - a "Memory" can support resources from all 3 categories: buffers, attachments, all other textures
      uint8_t memory;
    } tiers;

    // Features
    struct {
      // Swap chain
      bool swapChain; // NRISwapChain
      bool presentFromCompute; // see "SwapChainDesc::queue"
      bool waitableSwapChain; // see "SwapChainDesc::waitable"
      bool resizableSwapChain; // swap chain can be resized without triggering an "OUT_OF_DATE" error

      // Multi view
      bool flexibleMultiview; // see "Multiview::FLEXIBLE"
      bool layerBasedMultiview; // see "Multiview::LAYRED_BASED"
      bool viewportBasedMultiview; // see "Multiview::VIEWPORT_BASED"

      // Texture compression
      bool textureCompressionBC; // all "BC" texture formats are supported
      bool textureCompressionETC2; // all "ETC2" texture formats are supported
      bool textureCompressionASTC; // all "ASTC" texture formats are supported

      // Shader bytecode
      bool shaderBytecodeDXBC; // DXBC can be passed to "ShaderDesc::bytecode"
      bool shaderBytecodeDXIL; // DXIL can be passed to "ShaderDesc::bytecode"
      bool shaderBytecodeSPIRV; // SPIRV can be passed to "ShaderDesc::bytecode", WGPU expects Vulkan 1.2 environment
      bool shaderBytecodeWGSL; // WGSL can be passed to "ShaderDesc::bytecode"

      // Queries
      bool occlusion; // see "QueryType::OCCLUSION"
      bool timestamp; // see "QueryType::TIMESTAMP"
      bool timestampCopyQueue;
      // see "QueryType::TIMESTAMP_COPY_QUEUE", see "other.timestampCopyQueueResolveOnCopyQueue"
      bool calibratedTimestamps; // see "GetCalibratedTimestamps"

      // Shading rate
      bool additionalShadingRates; // see "ShadingRate"
      bool sumShadingRateCombiner; // see "ShadingRateCombiner::SUM"

      // Clear
      bool rectColorClears; // see "CmdClearAttachments"
      bool rectDepthStencilClears; // see "CmdClearAttachments"

      // Resolve
      bool regionResolve; // see "CmdResolveTexture"
      bool resolveOpMinMax; // see "ResolveOp"

      // Pipeline cache
      bool pipelineCache; // "PipelineCache" support (NOP fallback if unsupported, except on error)
      bool pipelineCacheControl;
      // "FAIL_ON_CACHE_MISS" enforces "FAILURE", useful for platforms that prohibit runtime PSO compilation (e.g., Xbox GDK)

      // Other
      bool getMemoryDesc2; // "GetXxxMemoryDesc2" support (VK: requires "maintenance4", D3D: supported)
      bool enhancedBarriers; // VK: supported, D3D12: requires "AgilitySDK", D3D11: unsupported
      bool tessellationShader; // Tessellation control and evaluation shader stages
      bool geometryShader; // Geometry shader stage
      bool meshShader; // NRIMeshShader
      bool lowLatency; // NRILowLatency
      bool descriptorHeap; // NRIDescriptorHeap
      bool video; // NRIVideo
      bool componentSwizzle; // see "ComponentSwizzle" (unsupported only in D3D11)
      bool independentFrontAndBackStencilReferenceAndMasks; // see "StencilAttachmentDesc::back"
      bool filterOpMinMax; // see "FilterOp"
      bool constantAlphaBlendFactors; // see "BlendFactor::CONSTANT_ALPHA" and "BlendFactor::ONE_MINUS_CONSTANT_ALPHA"
      bool logicOp; // see "LogicOp"
      bool depthBoundsTest; // see "DepthAttachmentDesc::boundsTest"
      bool drawIndirectCount; // see "countBuffer" and "countBufferOffset"
      bool lineSmoothing; // see "RasterizationDesc::lineSmoothing"
      bool meshShaderPipelineStats; // see "PipelineStatisticsDesc"
      bool dynamicDepthBias; // see "CmdSetDepthBias"
      bool viewportOriginBottomLeft; // see "Viewport"
      bool pipelineStatistics; // see "QueryType::PIPELINE_STATISTICS"
      bool rootConstantsOffset; // see "SetRootConstantsDesc" (unsupported only in D3D11)
      bool nonConstantBufferRootDescriptorOffset; // see "SetRootDescriptorDesc" (unsupported only in D3D11)
      bool mutableDescriptorType; // see "DescriptorType::MUTABLE"
      bool extendedDynamicState;
      // VK: allows to use "VertexBufferDesc::stride" (dynamic) instead of "VertexStreamDesc::stride" (static). Widely supported
      bool unifiedTextureLayouts;
      // VK: allows to use "GENERAL" everywhere: https://docs.vulkan.org/refpages/latest/refpages/source/VK_KHR_unified_image_layouts.html
      bool resourceAliasing;
      // binding multiple distinct texture or buffer objects to overlap the same underlying memory allocation (unsupported only in D3D11)
    } features;

    // Shader features
    // https://github.com/Microsoft/DirectXShaderCompiler/blob/main/docs/SPIR-V.rst
    struct {
      // Native types (I32 and F32 are always supported)
      // https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-scalar
      bool nativeI8; // "(u)int8_t"
      bool nativeI16; // "(u)int16_t"
      bool nativeF16; // "float16_t"
      bool nativeI64; // "(u)int64_t"
      bool nativeF64; // "double"

      // Atomics on native types (I32 atomics are always supported, for others it can be partial support of SMEM, texture or buffer atomics)
      // https://learn.microsoft.com/en-us/windows/win32/direct3d11/direct3d-11-advanced-stages-cs-atomic-functions
      // https://microsoft.github.io/DirectX-Specs/d3d/HLSL_SM_6_6_Int64_and_Float_Atomics.html
      bool atomicsI16; // "(u)int16_t" atomics
      bool atomicsF16; // "float16_t" atomics
      bool atomicsF32; // "float" atomics
      bool atomicsI64; // "(u)int64_t" atomics
      bool atomicsF64; // "double" atomics

      // Storage without format
      // https://learn.microsoft.com/en-us/windows/win32/direct3d12/typed-unordered-access-view-loads#using-unorm-and-snorm-typed-uav-loads-from-hlsl
      bool storageReadWithoutFormat; // NRI_FORMAT("unknown") is allowed for storage reads
      bool storageWriteWithoutFormat; // NRI_FORMAT("unknown") is allowed for storage writes

      // Wave intrinsics
      // https://github.com/microsoft/directxshadercompiler/wiki/wave-intrinsics
      bool waveQuery; // WaveIsFirstLane, WaveGetLaneCount, WaveGetLaneIndex
      bool waveVote; // WaveActiveAllTrue, WaveActiveAnyTrue, WaveActiveAllEqual
      bool waveShuffle; // WaveReadLaneFirst, WaveReadLaneAt
      bool waveArithmetic;
      // WaveActiveSum, WaveActiveProduct, WaveActiveMin, WaveActiveMax, WavePrefixProduct, WavePrefixSum
      bool waveReduction;
      // WaveActiveCountBits, WaveActiveBitAnd, WaveActiveBitOr, WaveActiveBitXor, WavePrefixCountBits
      bool waveQuad; // QuadReadLaneAt, QuadReadAcrossX, QuadReadAcrossY, QuadReadAcrossDiagonal

      // Other
      bool viewportIndex; // SV_ViewportArrayIndex, always can be used in geometry shaders
      bool layerIndex; // SV_RenderTargetArrayIndex, always can be used in geometry shaders
      bool unnormalizedCoordinates;
      // https://microsoft.github.io/DirectX-Specs/d3d/VulkanOn12.html#non-normalized-texture-sampling-coordinates
      bool clock; // https://github.com/Microsoft/DirectXShaderCompiler/blob/main/docs/SPIR-V.rst#readclock
      bool rasterizedOrderedView;
      // https://microsoft.github.io/DirectX-Specs/d3d/RasterOrderViews.html (aka fragment shader interlock)
      bool barycentric; // https://github.com/microsoft/DirectXShaderCompiler/wiki/SV_Barycentrics
      bool rayTracingPositionFetch;
      // https://docs.vulkan.org/features/latest/features/proposals/VK_KHR_ray_tracing_position_fetch.html
      bool integerDotProduct; // https://github.com/microsoft/DirectXShaderCompiler/wiki/Shader-Model-6.4
      bool inputAttachments;
      // https://github.com/Microsoft/DirectXShaderCompiler/blob/main/docs/SPIR-V.rst#subpass-inputs

      bool drawParameters;
      // GAPI-independent "NRI_BASE_VERTEX", "NRI_BASE_INSTANCE", "NRI_VERTEX_ID_OFFSET" and "NRI_INSTANCE_ID_OFFSET" (see "NRI.hlsl" for expected usage)
      bool drawIndex; // GAPI-independent "NRI_DRAW_ID" (see "NRI.hlsl" for expected usage)
    } shaderFeatures;

    // Video
    struct {
      struct {
        bool H264;
        bool H265;
        bool AV1;
      } decode;

      struct {
        bool H264;
        bool H265;
        bool AV1;
      } encode;
    } videoFeatures;
  };
#pragma endregion
}
