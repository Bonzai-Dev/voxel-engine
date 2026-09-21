#pragma once
#include <cstdint>
#include <cstring>
#include <cstddef>
#include <array>
#include <core/core.hpp>
#include <core/memory/intrusive_ptr.hpp>

/*
Overview:
 - Generalized common denominator for VK, D3D12 and D3D11
    - VK spec: https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html
       - Best practices: https://developer.nvidia.com/blog/vulkan-dos-donts/
       - Feature support coverage: https://vulkan.gpuinfo.org/
    - D3D12 spec: https://microsoft.github.io/DirectX-Specs/
       - Feature support coverage: https://d3d12infodb.boolka.dev/
    - D3D11 spec: https://microsoft.github.io/DirectX-Specs/d3d/archive/D3D11_3_FunctionalSpec.htm

Goals:
 - a Graphics API, not RHI!
 - generalization and unification of D3D12 and VK
 - explicitness (providing access to low-level features of modern GAPIs)
 - quality-of-life and high-level extensions (e.g., streaming and upscaling)
 - low overhead
 - cross-platform and platform independence (AMD/INTEL friendly)
 - D3D11 support (as much as possible)

Non-goals:
 - exposing entities not existing in GAPIs
 - high-level (D3D11-like) abstraction
 - hidden management of any kind (except for some high-level extensions where it's desired)
 - automatic barriers (better handled in a higher-level abstraction)

Thread safety:
 - Threadsafe: yes - free-threaded access
 - Threadsafe: no  - external synchronization required, i.e. one thread at a time (additional restrictions can apply)
 - Threadsafe: ?   - unclear status

Implicit:
 - Create*         - thread safe
 - Destroy*        - not thread safe (because of VK)
 - Cmd*            - not thread safe
*/

namespace Core::RHI {
  struct SwapChainInterface;
  struct SwapChainInfo;
  class SwapChain;

  constexpr uint32_t invalidQueueFamilyIndex = static_cast<uint32_t>(-1);
  constexpr uint32_t maxPhysicalDevicesCount = 32;
  constexpr uint32_t presentTimeout = 1000u; // 1 second
  constexpr uint32_t fenceTimeout = 5000u;

  class Resource: public RefCounted {
    public:
      Resource() = default;

      ~Resource() override = default;

      unsigned long addRef() override {
        return ++referenceCount;
      }

      unsigned long release() override {
        unsigned long result = --referenceCount;
        if (result == 0) {
          delete this;
        }
        return result;
      }
  };

  class Texture: public Resource {
    public:
      Texture() = default;

      ~Texture() override = default;
  };

  class DescriptorSet {
  };

  class Descriptor {
  };

  class Queue {
  };

  class PipelineLayout {
  };

  class PipelineCache {
  };

  class Pipeline {
  };

  class Buffer {
  };

/* clang-format off */
//============================================================================================================================================================================================
#pragma region [ Common ]
  //============================================================================================================================================================================================
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

  constexpr const char *resultToString(Result result) {
    switch (result) {
      case Result::DeviceLost: return "Device lost";
      case Result::OutOfDate: return "Out of date";
      case Result::InvalidSDK: return "Invalid SDK";
      case Result::Success: return "Success";
      case Result::Failure: return "Failure";
      case Result::InvalidArgument: return "Invalid argument";
      case Result::OutOfMemory: return "Out of memory";
      case Result::Unsupported: return "Unsupported";
      default: return "Unknown result";
    }
  }

  struct DepthStencil {
    float depth;
    uint8_t stencil;
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

  union Color {
    Color32f f;
    Color32ui ui;
    Color32i i;
  };

  union ClearValue {
    DepthStencil depthStencil;
    Color color;
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

  struct FormatProperties {
    const char* name;       // format name
    DataFormat format;          // self
    uint8_t redBits;        // R (or depth) bits
    uint8_t greenBits;      // G (or stencil) bits (0 if channels < 2)
    uint8_t blueBits;       // B bits (0 if channels < 3)
    uint8_t alphaBits;      // A (or shared exponent) bits (0 if channels < 4)
    uint8_t stride;         // block size in bytes
    uint8_t blockWidth;     // 1 for plain formats, >1 for compressed
    uint8_t blockHeight;    // 1 for plain formats, >1 for compressed
    bool isBgr;             // reversed channels (RGBA => BGRA)
    bool isCompressed;      // block-compressed format
    bool isDepth;           // has depth component
    bool isExpShared;       // shared exponent in alpha channel
    bool isFloat;           // floating point
    bool isPacked;          // 16- or 32- bit packed
    bool isInteger;         // integer
    bool isNorm;            // [0; 1] normalized
    bool isSigned;          // signed
    bool isSrgb;            // sRGB
    bool isStencil;         // has stencil component
  };

  // A bit represents a feature, supported by a format
  // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_feature_data_format_support
  // https://docs.vulkan.org/refpages/latest/refpages/source/VkFormatFeatureFlagBits2.html
  // WGPU: typed buffer views are unsupported; storage textures cannot be multisampled
  ENGINE_BITS(FormatSupportBits, uint16_t,
    Unsupported                     = 0,            // format is unsupported

    // Texture
    Texture                         = ENGINE_BIT(0),    // sampled texture view
    StorageTexture                  = ENGINE_BIT(1),    // storage texture view
    StorageTextureAtomics           = ENGINE_BIT(2),    // storage texture atomics other than Load / Store
    ColorAttachment                 = ENGINE_BIT(3),    // color attachment view
    DepthStencilAttachment          = ENGINE_BIT(4),    // depth-stencil attachment view
    Blend                           = ENGINE_BIT(5),    // color attachment blending
    Multisample2X                   = ENGINE_BIT(6),    // 2x multisampled texture
    MultiSample4X                   = ENGINE_BIT(7),    // 4x multisampled texture
    MultiSample8X                   = ENGINE_BIT(8),    // 8x multisampled texture
    MultiSampleResolve              = ENGINE_BIT(9),    // resolve source/destination

    // Buffer
    Buffer                          = ENGINE_BIT(10),   // typed buffer view
    StorageBuffer                   = ENGINE_BIT(11),   // typed storage buffer view
    StorageBufferAtomics            = ENGINE_BIT(12),   // typed storage buffer atomics other than Load / Store
    VertexBuffer                    = ENGINE_BIT(13),   // vertex buffer attribute

    // Texture / buffer
    StorageReadWithoutFormat        = ENGINE_BIT(14),   // storage read with unknown format
    StorageWriteWithoutFormat       = ENGINE_BIT(15)    // storage write with unknown format
  );

#define _ 0
#define X 1
  constexpr std::array<FormatProperties, static_cast<size_t>(DataFormat::Count)> formatProperties = {{
    //                                                                                                               isStencil
    //                                                                                                               isSrgb  |
    //                                                                                                          isSigned  |  |
    //                                                                                                         isNorm  |  |  |
    //                                                                                                   isInteger  |  |  |  |
    //                                                                                                 isPacked  |  |  |  |  |
    //                                                                                               isFloat  |  |  |  |  |  |
    //                                                                                        isExpShared  |  |  |  |  |  |  |
    //                                                                                         isDepth  |  |  |  |  |  |  |  |
    //                                                                                 isCompressed  |  |  |  |  |  |  |  |  |
    //                                                                                     isBgr  |  |  |  |  |  |  |  |  |  |
    //                                                                           blockHeight   |  |  |  |  |  |  |  |  |  |  |
    //                                                                        blockWidth   |   |  |  |  |  |  |  |  |  |  |  |
    //                                                                        stride   |   |   |  |  |  |  |  |  |  |  |  |  |
    //                                                                    A bits   |   |   |   |  |  |  |  |  |  |  |  |  |  |
    //                                                                B bits   |   |   |   |   |  |  |  |  |  |  |  |  |  |  |
    //                                                            G bits   |   |   |   |   |   |  |  |  |  |  |  |  |  |  |  |
    //                                                        R bits   |   |   |   |   |   |   |  |  |  |  |  |  |  |  |  |  |
    //                          self                               |   |   |   |   |   |   |   |  |  |  |  |  |  |  |  |  |  |
    // format name              |                                  |   |   |   |   |   |   |   |  |  |  |  |  |  |  |  |  |  |
    {"Unknown",                 DataFormat::Unknown,                   0,  0,  0,  0,  1,  0,  0,  _, _, _, _, _, _, _, _, _, _, _}, // UNKNOWN
    //                                                             r   g   b   a   s   w   h   b  c  d  e  f  p  i  n  s  s  s
    {"R8_UNORM",                DataFormat::R8_UNORM,                  8,  0,  0,  0,  1,  1,  1,  _, _, _, _, _, _, _, X, _, _, _}, // R8_UNORM
    {"R8_SNORM",                DataFormat::R8_SNORM,                  8,  0,  0,  0,  1,  1,  1,  _, _, _, _, _, _, _, X, X, _, _}, // R8_SNORM
    {"R8_UINT",                 DataFormat::R8_UINT,                   8,  0,  0,  0,  1,  1,  1,  _, _, _, _, _, _, X, _, _, _, _}, // R8_UINT
    {"R8_SINT",                 DataFormat::R8_SINT,                   8,  0,  0,  0,  1,  1,  1,  _, _, _, _, _, _, X, _, X, _, _}, // R8_SINT
    //                                                             r   g   b   a   s   w   h   b  c  d  e  f  p  i  n  s  s  s
    {"RG8_UNORM",               DataFormat::RG8_UNORM,                 8,  8,  0,  0,  2,  1,  1,  _, _, _, _, _, _, _, X, _, _, _}, // RG8_UNORM
    {"RG8_SNORM",               DataFormat::RG8_SNORM,                 8,  8,  0,  0,  2,  1,  1,  _, _, _, _, _, _, _, X, X, _, _}, // RG8_SNORM
    {"RG8_UINT",                DataFormat::RG8_UINT,                  8,  8,  0,  0,  2,  1,  1,  _, _, _, _, _, _, X, _, _, _, _}, // RG8_UINT
    {"RG8_SINT",                DataFormat::RG8_SINT,                  8,  8,  0,  0,  2,  1,  1,  _, _, _, _, _, _, X, _, X, _, _}, // RG8_SINT
    //                                                             r   g   b   a   s   w   h   b  c  d  e  f  p  i  n  s  s  s
    {"BGRA8_UNORM",             DataFormat::BGRA8_UNORM,               8,  8,  8,  8,  4,  1,  1,  X, _, _, _, _, _, _, X, _, _, _}, // BGRA8_UNORM
    {"BGRA8_SRGB",              DataFormat::BGRA8_SRGB,                8,  8,  8,  8,  4,  1,  1,  X, _, _, _, _, _, _, _, _, X, _}, // BGRA8_SRGB
    //                                                             r   g   b   a   s   w   h   b  c  d  e  f  p  i  n  s  s  s
    {"RGBA8_UNORM",             DataFormat::RGBA8_UNORM,               8,  8,  8,  8,  4,  1,  1,  _, _, _, _, _, _, _, X, _, _, _}, // RGBA8_UNORM
    {"RGBA8_SRGB",              DataFormat::RGBA8_SRGB,                8,  8,  8,  8,  4,  1,  1,  _, _, _, _, _, _, _, _, _, X, _}, // RGBA8_SRGB
    {"RGBA8_SNORM",             DataFormat::RGBA8_SNORM,               8,  8,  8,  8,  4,  1,  1,  _, _, _, _, _, _, _, X, X, _, _}, // RGBA8_SNORM
    {"RGBA8_UINT",              DataFormat::RGBA8_UINT,                8,  8,  8,  8,  4,  1,  1,  _, _, _, _, _, _, X, _, _, _, _}, // RGBA8_UINT
    {"RGBA8_SINT",              DataFormat::RGBA8_SINT,                8,  8,  8,  8,  4,  1,  1,  _, _, _, _, _, _, X, _, X, _, _}, // RGBA8_SINT
    //                                                             r   g   b   a   s   w   h   b  c  d  e  f  p  i  n  s  s  s
    {"R16_UNORM",               DataFormat::R16_UNORM,                 16, 0,  0,  0,  2,  1,  1,  _, _, _, _, _, _, _, X, _, _, _}, // R16_UNORM
    {"R16_SNORM",               DataFormat::R16_SNORM,                 16, 0,  0,  0,  2,  1,  1,  _, _, _, _, _, _, _, X, X, _, _}, // R16_SNORM
    {"R16_UINT",                DataFormat::R16_UINT,                  16, 0,  0,  0,  2,  1,  1,  _, _, _, _, _, _, X, _, _, _, _}, // R16_UINT
    {"R16_SINT",                DataFormat::R16_SINT,                  16, 0,  0,  0,  2,  1,  1,  _, _, _, _, _, _, X, _, X, _, _}, // R16_SINT
    {"R16_SFLOAT",              DataFormat::R16_SFLOAT,                16, 0,  0,  0,  2,  1,  1,  _, _, _, _, X, _, _, _, X, _, _}, // R16_SFLOAT
    //                                                             r   g   b   a   s   w   h   b  c  d  e  f  p  i  n  s  s  s
    {"RG16_UNORM",              DataFormat::RG16_UNORM,                16, 16, 0,  0,  4,  1,  1,  _, _, _, _, _, _, _, X, _, _, _}, // RG16_UNORM
    {"RG16_SNORM",              DataFormat::RG16_SNORM,                16, 16, 0,  0,  4,  1,  1,  _, _, _, _, _, _, _, X, X, _, _}, // RG16_SNORM
    {"RG16_UINT",               DataFormat::RG16_UINT,                 16, 16, 0,  0,  4,  1,  1,  _, _, _, _, _, _, X, _, _, _, _}, // RG16_UINT
    {"RG16_SINT",               DataFormat::RG16_SINT,                 16, 16, 0,  0,  4,  1,  1,  _, _, _, _, _, _, X, _, X, _, _}, // RG16_SINT
    {"RG16_SFLOAT",             DataFormat::RG16_SFLOAT,               16, 16, 0,  0,  4,  1,  1,  _, _, _, _, X, _, _, _, X, _, _}, // RG16_SFLOAT
    //                                                             r   g   b   a   s   w   h   b  c  d  e  f  p  i  n  s  s  s
    {"RGBA16_UNORM",            DataFormat::RGBA16_UNORM,              16, 16, 16, 16, 8,  1,  1,  _, _, _, _, _, _, _, X, _, _, _}, // RGBA16_UNORM
    {"RGBA16_SNORM",            DataFormat::RGBA16_SNORM,              16, 16, 16, 16, 8,  1,  1,  _, _, _, _, _, _, _, X, X, _, _}, // RGBA16_SNORM
    {"RGBA16_UINT",             DataFormat::RGBA16_UINT,               16, 16, 16, 16, 8,  1,  1,  _, _, _, _, _, _, X, _, _, _, _}, // RGBA16_UINT
    {"RGBA16_SINT",             DataFormat::RGBA16_SINT,               16, 16, 16, 16, 8,  1,  1,  _, _, _, _, _, _, X, _, X, _, _}, // RGBA16_SINT
    {"RGBA16_SFLOAT",           DataFormat::RGBA16_SFLOAT,             16, 16, 16, 16, 8,  1,  1,  _, _, _, _, X, _, _, _, X, _, _}, // RGBA16_SFLOAT
    //                                                             r   g   b   a   s   w   h   b  c  d  e  f  p  i  n  s  s  s
    {"R32_UINT",                DataFormat::R32_UINT,                  32, 0,  0,  0,  4,  1,  1,  _, _, _, _, _, _, X, _, _, _, _}, // R32_UINT
    {"R32_SINT",                DataFormat::R32_SINT,                  32, 0,  0,  0,  4,  1,  1,  _, _, _, _, _, _, X, _, X, _, _}, // R32_SINT
    {"R32_SFLOAT",              DataFormat::R32_SFLOAT,                32, 0,  0,  0,  4,  1,  1,  _, _, _, _, X, _, _, _, X, _, _}, // R32_SFLOAT
    //                                                             r   g   b   a   s   w   h   b  c  d  e  f  p  i  n  s  s  s
    {"RG32_UINT",               DataFormat::RG32_UINT,                 32, 32, 0,  0,  8,  1,  1,  _, _, _, _, _, _, X, _, _, _, _}, // RG32_UINT
    {"RG32_SINT",               DataFormat::RG32_SINT,                 32, 32, 0,  0,  8,  1,  1,  _, _, _, _, _, _, X, _, X, _, _}, // RG32_SINT
    {"RG32_SFLOAT",             DataFormat::RG32_SFLOAT,               32, 32, 0,  0,  8,  1,  1,  _, _, _, _, X, _, _, _, X, _, _}, // RG32_SFLOAT
    //                                                             r   g   b   a   s   w   h   b  c  d  e  f  p  i  n  s  s  s
    {"RGB32_UINT",              DataFormat::RGB32_UINT,                32, 32, 32, 0,  12, 1,  1,  _, _, _, _, _, _, X, _, _, _, _}, // RGB32_UINT
    {"RGB32_SINT",              DataFormat::RGB32_SINT,                32, 32, 32, 0,  12, 1,  1,  _, _, _, _, _, _, X, _, X, _, _}, // RGB32_SINT
    {"RGB32_SFLOAT",            DataFormat::RGB32_SFLOAT,              32, 32, 32, 0,  12, 1,  1,  _, _, _, _, X, _, _, _, X, _, _}, // RGB32_SFLOAT
    //                                                             r   g   b   a   s   w   h   b  c  d  e  f  p  i  n  s  s  s
    {"RGBA32_UINT",             DataFormat::RGBA32_UINT,               32, 32, 32, 32, 16, 1,  1,  _, _, _, _, _, _, X, _, _, _, _}, // RGBA32_UINT
    {"RGBA32_SINT",             DataFormat::RGBA32_SINT,               32, 32, 32, 32, 16, 1,  1,  _, _, _, _, _, _, X, _, X, _, _}, // RGBA32_SINT
    {"RGBA32_SFLOAT",           DataFormat::RGBA32_SFLOAT,             32, 32, 32, 32, 16, 1,  1,  _, _, _, _, X, _, _, _, X, _, _}, // RGBA32_SFLOAT
    //                                                             r   g   b   a   s   w   h   b  c  d  e  f  p  i  n  s  s  s
    {"B5_G6_R5_UNORM",          DataFormat::B5_G6_R5_UNORM,            5,  6,  5,  0,  2,  1,  1,  X, _, _, _, _, X, _, X, _, _, _}, // B5_G6_R5_UNORM
    {"B5_G5_R5_A1_UNORM",       DataFormat::B5_G5_R5_A1_UNORM,         5,  5,  5,  1,  2,  1,  1,  X, _, _, _, _, X, _, X, _, _, _}, // B5_G5_R5_A1_UNORM
    {"B4_G4_R4_A4_UNORM",       DataFormat::B4_G4_R4_A4_UNORM,         4,  4,  4,  4,  2,  1,  1,  X, _, _, _, _, X, _, X, _, _, _}, // B4_G4_R4_A4_UNORM
    {"R10_G10_B10_A2_UNORM",    DataFormat::R10_G10_B10_A2_UNORM,      10, 10, 10, 2,  4,  1,  1,  _, _, _, _, _, X, _, X, _, _, _}, // R10_G10_B10_A2_UNORM
    {"R10_G10_B10_A2_UINT",     DataFormat::R10_G10_B10_A2_UINT,       10, 10, 10, 2,  4,  1,  1,  _, _, _, _, _, X, X, _, _, _, _}, // R10_G10_B10_A2_UINT
    {"R11_G11_B10_UFLOAT",      DataFormat::R11_G11_B10_UFLOAT,        11, 11, 10, 0,  4,  1,  1,  _, _, _, _, X, X, _, _, _, _, _}, // R11_G11_B10_UFLOAT
    {"R9_G9_B9_E5_UFLOAT",      DataFormat::R9_G9_B9_E5_UFLOAT,        9,  9,  9,  5,  4,  1,  1,  _, _, _, X, X, X, _, _, _, _, _}, // R9_G9_B9_E5_UFLOAT
    //                                                             r   g   b   a   s   w   h   b   c  d  e  f  p  i  n  s  s  s
    {"BC1_RGBA_UNORM",          DataFormat::BC1_RGBA_UNORM,            5,  6,  5,  1,  8,  4,  4,  _, X, _, _, _, _, _, X, _, _, _}, // BC1_RGBA_UNORM
    {"BC1_RGBA_SRGB",           DataFormat::BC1_RGBA_SRGB,             5,  6,  5,  1,  8,  4,  4,  _, X, _, _, _, _, _, _, _, X, _}, // BC1_RGBA_SRGB
    {"BC2_RGBA_UNORM",          DataFormat::BC2_RGBA_UNORM,            5,  6,  5,  4,  16, 4,  4,  _, X, _, _, _, _, _, X, _, _, _}, // BC2_RGBA_UNORM
    {"BC2_RGBA_SRGB",           DataFormat::BC2_RGBA_SRGB,             5,  6,  5,  4,  16, 4,  4,  _, X, _, _, _, _, _, _, _, X, _}, // BC2_RGBA_SRGB
    {"BC3_RGBA_UNORM",          DataFormat::BC3_RGBA_UNORM,            5,  6,  5,  8,  16, 4,  4,  _, X, _, _, _, _, _, X, _, _, _}, // BC3_RGBA_UNORM
    {"BC3_RGBA_SRGB",           DataFormat::BC3_RGBA_SRGB,             5,  6,  5,  8,  16, 4,  4,  _, X, _, _, _, _, _, _, _, X, _}, // BC3_RGBA_SRGB
    {"BC4_R_UNORM",             DataFormat::BC4_R_UNORM,               8,  0,  0,  0,  8,  4,  4,  _, X, _, _, _, _, _, X, _, _, _}, // BC4_R_UNORM
    {"BC4_R_SNORM",             DataFormat::BC4_R_SNORM,               8,  0,  0,  0,  8,  4,  4,  _, X, _, _, _, _, _, X, X, _, _}, // BC4_R_SNORM
    {"BC5_RG_UNORM",            DataFormat::BC5_RG_UNORM,              8,  8,  0,  0,  16, 4,  4,  _, X, _, _, _, _, _, X, _, _, _}, // BC5_RG_UNORM
    {"BC5_RG_SNORM",            DataFormat::BC5_RG_SNORM,              8,  8,  0,  0,  16, 4,  4,  _, X, _, _, _, _, _, X, X, _, _}, // BC5_RG_SNORM
    {"BC6H_RGB_UFLOAT",         DataFormat::BC6H_RGB_UFLOAT,           16, 16, 16, 0,  16, 4,  4,  _, X, _, _, X, _, _, _, _, _, _}, // BC6H_RGB_UFLOAT
    {"BC6H_RGB_SFLOAT",         DataFormat::BC6H_RGB_SFLOAT,           16, 16, 16, 0,  16, 4,  4,  _, X, _, _, X, _, _, _, X, _, _}, // BC6H_RGB_SFLOAT
    {"BC7_RGBA_UNORM",          DataFormat::BC7_RGBA_UNORM,            8,  8,  8,  8,  16, 4,  4,  _, X, _, _, _, _, _, X, _, _, _}, // BC7_RGBA_UNORM
    {"BC7_RGBA_SRGB",           DataFormat::BC7_RGBA_SRGB,             8,  8,  8,  8,  16, 4,  4,  _, X, _, _, _, _, _, _, _, X, _}, // BC7_RGBA_SRGB
    //                                                             r   g   b   a   s   w   h   b  c  d  e  f  p  i  n  s  s  s
    {"ETC2_RGB8_UNORM",         DataFormat::ETC2_RGB8_UNORM,           8,  8,  8,  0,  8,  4,  4,  _, X, _, _, _, _, _, X, _, _, _}, // ETC2_RGB8_UNORM
    {"ETC2_RGB8_SRGB",          DataFormat::ETC2_RGB8_SRGB,            8,  8,  8,  0,  8,  4,  4,  _, X, _, _, _, _, _, _, _, X, _}, // ETC2_RGB8_SRGB
    {"ETC2_RGB8_A1_UNORM",      DataFormat::ETC2_RGB8_A1_UNORM,        8,  8,  8,  1,  8,  4,  4,  _, X, _, _, _, _, _, X, _, _, _}, // ETC2_RGB8_A1_UNORM
    {"ETC2_RGB8_A1_SRGB",       DataFormat::ETC2_RGB8_A1_SRGB,         8,  8,  8,  1,  8,  4,  4,  _, X, _, _, _, _, _, _, _, X, _}, // ETC2_RGB8_A1_SRGB
    {"ETC2_RGB8_A8_UNORM",      DataFormat::ETC2_RGB8_A8_UNORM,        8,  8,  8,  8,  16, 4,  4,  _, X, _, _, _, _, _, X, _, _, _}, // ETC2_RGB8_A8_UNORM
    {"ETC2_RGB8_A8_SRGB",       DataFormat::ETC2_RGB8_A8_SRGB,         8,  8,  8,  8,  16, 4,  4,  _, X, _, _, _, _, _, _, _, X, _}, // ETC2_RGB8_A8_SRGB
    {"ETC2_R11_UNORM",          DataFormat::ETC2_R11_UNORM,            11, 0,  0,  0,  8,  4,  4,  _, X, _, _, _, _, _, X, _, _, _}, // ETC2_R11_UNORM
    {"ETC2_R11_SNORM",          DataFormat::ETC2_R11_SNORM,            11, 0,  0,  0,  8,  4,  4,  _, X, _, _, _, _, _, X, X, _, _}, // ETC2_R11_SNORM
    {"ETC2_R11_G11_UNORM",      DataFormat::ETC2_R11_G11_UNORM,        11, 11, 0,  0,  16, 4,  4,  _, X, _, _, _, _, _, X, _, _, _}, // ETC2_R11_G11_UNORM
    {"ETC2_R11_G11_SNORM",      DataFormat::ETC2_R11_G11_SNORM,        11, 11, 0,  0,  16, 4,  4,  _, X, _, _, _, _, _, X, X, _, _}, // ETC2_R11_G11_SNORM
    //                                                             r   g   b   a   s   w   h   b  c  d  e  f  p  i  n  s  s  s
    {"ASTC_4X4_UNORM",          DataFormat::ASTC_4X4_UNORM,            8,  8,  8,  8,  16, 4,  4,  _, X, _, _, _, _, _, X, _, _, _}, // ASTC_4X4_UNORM
    {"ASTC_4X4_SRGB",           DataFormat::ASTC_4X4_SRGB,             8,  8,  8,  8,  16, 4,  4,  _, X, _, _, _, _, _, _, _, X, _}, // ASTC_4X4_SRGB
    {"ASTC_5X4_UNORM",          DataFormat::ASTC_5X4_UNORM,            8,  8,  8,  8,  16, 5,  4,  _, X, _, _, _, _, _, X, _, _, _}, // ASTC_5X4_UNORM
    {"ASTC_5X4_SRGB",           DataFormat::ASTC_5X4_SRGB,             8,  8,  8,  8,  16, 5,  4,  _, X, _, _, _, _, _, _, _, X, _}, // ASTC_5X4_SRGB
    {"ASTC_5X5_UNORM",          DataFormat::ASTC_5X5_UNORM,            8,  8,  8,  8,  16, 5,  5,  _, X, _, _, _, _, _, X, _, _, _}, // ASTC_5X5_UNORM
    {"ASTC_5X5_SRGB",           DataFormat::ASTC_5X5_SRGB,             8,  8,  8,  8,  16, 5,  5,  _, X, _, _, _, _, _, _, _, X, _}, // ASTC_5X5_SRGB
    {"ASTC_6X5_UNORM",          DataFormat::ASTC_6X5_UNORM,            8,  8,  8,  8,  16, 6,  5,  _, X, _, _, _, _, _, X, _, _, _}, // ASTC_6X5_UNORM
    {"ASTC_6X5_SRGB",           DataFormat::ASTC_6X5_SRGB,             8,  8,  8,  8,  16, 6,  5,  _, X, _, _, _, _, _, _, _, X, _}, // ASTC_6X5_SRGB
    {"ASTC_6X6_UNORM",          DataFormat::ASTC_6X6_UNORM,            8,  8,  8,  8,  16, 6,  6,  _, X, _, _, _, _, _, X, _, _, _}, // ASTC_6X6_UNORM
    {"ASTC_6X6_SRGB",           DataFormat::ASTC_6X6_SRGB,             8,  8,  8,  8,  16, 6,  6,  _, X, _, _, _, _, _, _, _, X, _}, // ASTC_6X6_SRGB
    {"ASTC_8X5_UNORM",          DataFormat::ASTC_8X5_UNORM,            8,  8,  8,  8,  16, 8,  5,  _, X, _, _, _, _, _, X, _, _, _}, // ASTC_8X5_UNORM
    {"ASTC_8X5_SRGB",           DataFormat::ASTC_8X5_SRGB,             8,  8,  8,  8,  16, 8,  5,  _, X, _, _, _, _, _, _, _, X, _}, // ASTC_8X5_SRGB
    {"ASTC_8X6_UNORM",          DataFormat::ASTC_8X6_UNORM,            8,  8,  8,  8,  16, 8,  6,  _, X, _, _, _, _, _, X, _, _, _}, // ASTC_8X6_UNORM
    {"ASTC_8X6_SRGB",           DataFormat::ASTC_8X6_SRGB,             8,  8,  8,  8,  16, 8,  6,  _, X, _, _, _, _, _, _, _, X, _}, // ASTC_8X6_SRGB
    {"ASTC_8X8_UNORM",          DataFormat::ASTC_8X8_UNORM,            8,  8,  8,  8,  16, 8,  8,  _, X, _, _, _, _, _, X, _, _, _}, // ASTC_8X8_UNORM
    {"ASTC_8X8_SRGB",           DataFormat::ASTC_8X8_SRGB,             8,  8,  8,  8,  16, 8,  8,  _, X, _, _, _, _, _, _, _, X, _}, // ASTC_8X8_SRGB
    {"ASTC_10X5_UNORM",         DataFormat::ASTC_10X5_UNORM,           8,  8,  8,  8,  16, 10, 5,  _, X, _, _, _, _, _, X, _, _, _}, // ASTC_10X5_UNORM
    {"ASTC_10X5_SRGB",          DataFormat::ASTC_10X5_SRGB,            8,  8,  8,  8,  16, 10, 5,  _, X, _, _, _, _, _, _, _, X, _}, // ASTC_10X5_SRGB
    {"ASTC_10X6_UNORM",         DataFormat::ASTC_10X6_UNORM,           8,  8,  8,  8,  16, 10, 6,  _, X, _, _, _, _, _, X, _, _, _}, // ASTC_10X6_UNORM
    {"ASTC_10X6_SRGB",          DataFormat::ASTC_10X6_SRGB,            8,  8,  8,  8,  16, 10, 6,  _, X, _, _, _, _, _, _, _, X, _}, // ASTC_10X6_SRGB
    {"ASTC_10X8_UNORM",         DataFormat::ASTC_10X8_UNORM,           8,  8,  8,  8,  16, 10, 8,  _, X, _, _, _, _, _, X, _, _, _}, // ASTC_10X8_UNORM
    {"ASTC_10X8_SRGB",          DataFormat::ASTC_10X8_SRGB,            8,  8,  8,  8,  16, 10, 8,  _, X, _, _, _, _, _, _, _, X, _}, // ASTC_10X8_SRGB
    {"ASTC_10X10_UNORM",        DataFormat::ASTC_10X10_UNORM,          8,  8,  8,  8,  16, 10, 10, _, X, _, _, _, _, _, X, _, _, _}, // ASTC_10X10_UNORM
    {"ASTC_10X10_SRGB",         DataFormat::ASTC_10X10_SRGB,           8,  8,  8,  8,  16, 10, 10, _, X, _, _, _, _, _, _, _, X, _}, // ASTC_10X10_SRGB
    {"ASTC_12X10_UNORM",        DataFormat::ASTC_12X10_UNORM,          8,  8,  8,  8,  16, 12, 10, _, X, _, _, _, _, _, X, _, _, _}, // ASTC_12X10_UNORM
    {"ASTC_12X10_SRGB",         DataFormat::ASTC_12X10_SRGB,           8,  8,  8,  8,  16, 12, 10, _, X, _, _, _, _, _, _, _, X, _}, // ASTC_12X10_SRGB
    {"ASTC_12X12_UNORM",        DataFormat::ASTC_12X12_UNORM,          8,  8,  8,  8,  16, 12, 12, _, X, _, _, _, _, _, X, _, _, _}, // ASTC_12X12_UNORM
    {"ASTC_12X12_SRGB",         DataFormat::ASTC_12X12_SRGB,           8,  8,  8,  8,  16, 12, 12, _, X, _, _, _, _, _, _, _, X, _}, // ASTC_12X12_SRGB
    //                                                             r   g   b   a   s   w   h   b  c  d  e  f  p  i  n  s  s  s
    {"D16_UNORM",               DataFormat::D16_UNORM,                 16, 0,  0,  0,  2,  1,  1,  _, _, X, _, _, _, _, X, _, _, _}, // D16_UNORM
    {"D32_SFLOAT",              DataFormat::D32_SFLOAT,                32, 0,  0,  0,  4,  1,  1,  _, _, X, _, X, _, _, _, X, _, _}, // D32_SFLOAT
    {"D24_UNORM_S8_UINT",       DataFormat::D24_UNORM_S8_UINT,         24, 8,  0,  0,  4,  1,  1,  _, _, X, _, _, _, X, X, _, _, X}, // D24_UNORM_S8_UINT
    {"D32_SFLOAT_S8_UINT",      DataFormat::D32_SFLOAT_S8_UINT,        32, 8,  0,  0,  8,  1,  1,  _, _, X, _, X, _, X, _, X, _, X}, // D32_SFLOAT_S8_UINT
  }};
#undef _
#undef X

  inline const FormatProperties &getFormatProperties(DataFormat format) {
    return formatProperties[static_cast<size_t>(format)];
  }

  // https://learn.microsoft.com/en-us/windows/win32/direct3d12/subresources#plane-slice
  // https://docs.vulkan.org/refpages/latest/refpages/source/VkImageAspectFlagBits.html
  ENGINE_BITS(PlaneBits, uint8_t,
    All                             = 0,            // lazy default
    None                            = ENGINE_BIT(7),    // no accessible planes (needed for a read-only depth-stencil attachment)

    Color                           = ENGINE_BIT(0),    // indicates "color" plane (same as "ALL" for color formats)

    // D3D11: can't be addressed individually in "copy" and "resolve" operations
    Depth                           = ENGINE_BIT(1),    // indicates "depth" plane (same as "ALL" for depth-only formats)
    Stencil                         = ENGINE_BIT(2)     // indicates "stencil" plane in depth-stencil formats
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
    All                             = 0,            // Lazy default for barriers                          Shader stage
    None                            = 0x7FFFFFFF,

    // Graphics                                    // Invoked by "CmdDraw*"
    IndexInput                      = ENGINE_BIT(0),    //    Index buffer consumption
    VertexShader                    = ENGINE_BIT(1),    //    Vertex shader                                   X (required within GRAPHICS bind point)
    TessellationControlShader       = ENGINE_BIT(2),    //    Tessellation control (hull) shader              X
    TessellationEvaluationShader    = ENGINE_BIT(3),    //    Tessellation evaluation (domain) shader         X
    GeometryShader                  = ENGINE_BIT(4),    //    Geometry shader                                 X
    TaskShader                      = ENGINE_BIT(5),    //    Task (amplification) shader                     X
    MeshShader                      = ENGINE_BIT(6),    //    Mesh shader                                     X (or required within GRAPHICS bind point)
    FragmentShader                  = ENGINE_BIT(7),    //    Fragment (pixel) shader                         X
    DepthStencilAttachment          = ENGINE_BIT(8),    //    Depth-stencil R/W operations
    ColorAttachment                 = ENGINE_BIT(9),    //    Color R/W operations
    ShadingRateAttachment           = ENGINE_BIT(10),   //    Shading rate attachment R

    // Compute                                     // Invoked by "CmdDispatch*" (not Rays)
    ComputeShader                   = ENGINE_BIT(11),   //    Compute shader                                  X (required within COMPUTE bind point)

    // Ray tracing                                 // Invoked by "CmdDispatchRays*"
    RayGenShader                    = ENGINE_BIT(12),   //    Ray generation shader                           X (required within RAY_TRACING bind point)
    MissShader                      = ENGINE_BIT(13),   //    Miss shader                                     X
    IntersectionShader              = ENGINE_BIT(14),   //    Intersection shader                             X
    ClosestHitShader                = ENGINE_BIT(15),   //    Closest hit shader                              X
    AnyHitShader                    = ENGINE_BIT(16),   //    Any hit shader                                  X
    CallableShader                  = ENGINE_BIT(17),   //    Callable shader                                 X
    AccelerationStructure           = ENGINE_BIT(18),   // Invoked by "Cmd*AccelerationStructure*" commands
    Micromap                        = ENGINE_BIT(19),   // Invoked by "Cmd*Micromap*" commands

    // Other
    Copy                            = ENGINE_BIT(20),   // Invoked by "CmdCopy*", "CmdUpload*" and "CmdReadback*"
    Resolve                         = ENGINE_BIT(21),   // Invoked by "CmdResolveTexture"
    ClearStorage                    = ENGINE_BIT(22),   // Invoked by "CmdClearStorage"

    // Modifiers
    Indirect                        = ENGINE_BIT(23),   // Invoked by "Indirect" commands (used in addition to other bits)

    // Umbrella stages
    TessellationShaders             = TessellationControlShader | TessellationEvaluationShader,

    MeshShaders                     = TaskShader | MeshShader,

    GraphicsShaders                 = VertexShader | TessellationShaders | GeometryShader | MeshShaders | FragmentShader,

    RayTracingShaders               = RayGenShader | MissShader | IntersectionShader | ClosestHitShader | AnyHitShader | CallableShader,

    AllShaders                      = GraphicsShaders | ComputeShader | RayTracingShaders,

    Graphics                        = IndexInput | GraphicsShaders | DepthStencilAttachment | ColorAttachment | ShadingRateAttachment
  );
#pragma endregion

//============================================================================================================================================================================================
#pragma region [ Resource views and samplers (descriptors) ]
//============================================================================================================================================================================================
  // https://microsoft.github.io/DirectX-Specs/d3d/ResourceBinding.html#creating-descriptors
  enum class TextureView: uint8_t {
    // Shader resources         // HLSL type                        Compatible "DescriptorType"     Compatible "TextureType"
    Texture,                        // Texture[1D/2D/3D](MS)            TEXTURE                         1D, 2D, 3D
    TextureArray,                  // Texture[1D/2D](MS)Array          TEXTURE                         1D, 2D
    TextureCube,                   // TextureCube                      TEXTURE                             2D
    TextureCubeArray,             // TextureCubeArray                 TEXTURE                             2D
    StorageTexture,                // RWTexture[1D/2D/3D](MS)          STORAGE_TEXTURE                 1D, 2D, 3D
    StorageTextureArray,          // RWTexture[1D/2D](MS)Array        STORAGE_TEXTURE                 1D, 2D
    SubpassInput,                  // SubpassInput(MS) (non-array)     INPUT_ATTACHMENT                    2D

    // Host-only
    ColorAttachment,               //                                                                  1D, 2D, 3D
    DepthStencilAttachment,       //                                                                  1D, 2D
    ShadingRateAttachment         //                                                                      2D
  };

  enum class BufferView: uint8_t {
    // Shader resources         // HLSL type                        Compatible "DescriptorType"
    Bugger,                         // Buffer                           BUFFER
    StructuredBuffer,              // StructuredBuffer                 STRUCTURED_BUFFER
    ByteAddressBuffer,            // ByteAddressBuffer                STRUCTURED_BUFFER
    StorageBuffer,                 // RWBuffer                         STORAGE_BUFFER
    StorageStructuredBuffer,      // RWStructuredBuffer               STORAGE_STRUCTURED_BUFFER
    StorageByteAddressBuffer,    // RWByteAddressBuffer              STORAGE_STRUCTURED_BUFFER
    ConstantBuffer                 // ConstantBuffer                   CONSTANT_BUFFER
  };

  // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_filter_reduction_type
  // https://docs.vulkan.org/refpages/latest/refpages/source/VkSamplerReductionMode.html
  enum class FilterOp: uint8_t {
    Average,    // a weighted average (sum) of values in the footprint (default)
    Min,        // a component-wise minimum of values in the footprint with non-zero weights, requires "features.filterOpMinMax"
    Max         // a component-wise maximum of values in the footprint with non-zero weights, requires "features.filterOpMinMax"
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

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkFilter.html
  // https://docs.vulkan.org/refpages/latest/refpages/source/VkSamplerMipmapMode.html
  // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_filter
  enum class Filter: uint8_t {
    Nearest,
    Linear
  };

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkCompareOp.html
  // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_comparison_func
  // R - fragment depth, stencil reference or "SampleCmp" reference
  // D - depth or stencil buffer
  enum class CompareOp: uint8_t {
    None,                       // test is disabled
    Always,                     // true
    Never,                      // false
    Equal,                      // R == D
    NotEqual,                  // R != D
    Less,                       // R < D
    LessEqual,                 // R <= D
    Greater,                    // R > D
    GreaterEqual,               // R >= D
    Count
  };

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkComponentSwizzle.html
  // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_shader_component_mapping
  enum class ComponentSwizzle: uint8_t {
    Identity,                   // format-specific default

    // Requires "features.componentSwizzle"
    Zero,                       // 0
    One,                        // 1 or 1.0
    R,                          // .x component (red)
    G,                          // .y component (green)
    B,                          // .z component (blue)
    A                           // .w component (alpha)
  };

  struct ComponentMapping {
    // Only for non-"STORAGE" views
    ComponentSwizzle r;
    ComponentSwizzle g;
    ComponentSwizzle b;
    ComponentSwizzle a;
  };

  struct TextureViewInfo {
    const Texture *texture;
    TextureView type;
    DataFormat format;
    uint16_t mipOffset;
    uint16_t mipNum;                      // can be "REMAINING"
    uint16_t layerOffset;
    uint16_t layerNum;                    // can be "REMAINING"
    uint16_t sliceOffset;
    uint16_t sliceNum;                    // can be "REMAINING"
    PlaneBits planes;                  // accessible planes (missing planes for a "DEPTH_STENCIL_ATTACHMENT" are considered read-only)
    ComponentMapping components;
  };

  struct BufferViewInfo {
    const Buffer *buffer;
    BufferView type;
    uint64_t offset;                        // expects "memoryAlignment.bufferShaderResourceOffset" for shader resources
    uint64_t size;                          // can be "WHOLE_SIZE"
    DataFormat format;         // needed for typed views, i.e. "BUFFER" and "STORAGE_BUFFER"
    uint32_t structureStride;   // needed for structured views, i.e. "STRUCTURED_BUFFER" and "STORAGE_STRUCTURED_BUFFER" (= "BufferDesc::structureStride", if not provided)
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
  struct SamplerInfo {
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
    IgnoreGlobalSpirvOffset = ENGINE_BIT(0), // VK: ignore "DeviceCreationDesc::vkBindingOffsets"
    EnableDrawParametersEmulation = ENGINE_BIT(1), // D3D12: enable draw parameters emulation, requires "shaderFeatures.drawParameters"
    EnableDrawIndexEmulation = ENGINE_BIT(2), // D3D12: enable draw index emulation, requires "shaderFeatures.drawIndex"

    // https://github.com/Microsoft/DirectXShaderCompiler/blob/main/docs/SPIR-V.rst#resourcedescriptorheaps--samplerdescriptorheaps
    // Default VK bindings can be changed via "-fvk-bind-sampler-heap" and "-fvk-bind-resource-heap" DXC options
    SamplerHeapDirectlyIndexed = ENGINE_BIT(3), // requires "shaderModel >= 66"
    ResourceHeapDirectlyIndexed = ENGINE_BIT(4) // requires "shaderModel >= 66"
  );

  ENGINE_BITS(DescriptorPoolBits, uint8_t,
      NONE                                    = 0,
      ALLOW_UPDATE_AFTER_SET                  = ENGINE_BIT(0)     // allows "DescriptorSetBits::ALLOW_UPDATE_AFTER_SET"
  );

  ENGINE_BITS(DescriptorSetBits, uint8_t,
      NONE                                    = 0,
      ALLOW_UPDATE_AFTER_SET                  = ENGINE_BIT(0)     // allows "DescriptorRangeBits::ALLOW_UPDATE_AFTER_SET"
  );

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkDescriptorBindingFlagBits.html
  ENGINE_BITS(DescriptorRangeBits, uint8_t,
      NONE                                    = 0,
      PARTIALLY_BOUND                         = ENGINE_BIT(0),    // descriptors in range may not contain valid descriptors at the time the descriptors are consumed (but referenced descriptors must be valid)
      ARRAY                                   = ENGINE_BIT(1),    // descriptors in range are organized into an array
      VARIABLE_SIZED_ARRAY                    = ENGINE_BIT(2),    // descriptors in range are organized into a variable-sized array, which size is specified via "variableDescriptorNum" argument of "AllocateDescriptorSets" function

      // https://docs.vulkan.org/samples/latest/samples/extensions/descriptor_indexing/README.html#_update_after_bind_streaming_descriptors_concurrently
      // WGPU: true "update after set" is unsupported because bind groups are immutable; "update + rebind" can work, but previously recorded commands can't be patched
      ALLOW_UPDATE_AFTER_SET                  = ENGINE_BIT(3)     // descriptors in range can be updated after "CmdSetDescriptorSet" but before "QueueSubmit", also works as "DATA_VOLATILE"
  );

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkDescriptorType.html
  enum DescriptorType: uint8_t {
    // Typed   HLSL reg    Compatible resources
    // Sampler heap
    Sampler,                    // -        s           sampler

    // Resource heap
    // - a mutable descriptor is a proxy "union" descriptor for all resource descriptor types, i.e. non-sampler
    // - a mutable descriptor can't be created, it can only be allocated from a pool (i.e. used in a "DescriptorRangeDesc")
    // - a mutable descriptor must "mutate" to any resource descriptor via "UpdateDescriptorRanges" or "CopyDescriptorRanges"
    // - a mutable descriptor range may include any non-sampler descriptors, which may be directly indexed in shaders
    Mutable,                    // -        -           any non-sampler

    // Optimized resources
    Texture,                    // +        t           TextureView: TEXTURE, TEXTURE_ARRAY, TEXTURE_CUBE, TEXTURE_CUBE_ARRAY
    StorageTexture,            // +        u           TextureView: STORAGE_TEXTURE, STORAGE_TEXTURE_ARRAY
    InputAttachment,           // +        -           TextureView: SUBPASS_INPUT

    Buffer,                     // +        t           BufferView: BUFFER
    StorageBuffer,             // +        u           BufferView: STORAGE_BUFFER
    ConstantBuffer,            // -        b           BufferView: CONSTANT_BUFFER
    StructuredBuffer,          // -        t           BufferView: STRUCTURED_BUFFER, BYTE_ADDRESS_BUFFER
    StorageStructuredBuffer,  // -        u           BufferView: STORAGE_STRUCTURED_BUFFER, STORAGE_BYTE_ADDRESS_BUFFER

    AccelerationStructure      // -        t           acceleration structure, requires "features.rayTracing"
  };

  // "DescriptorRange" consists of "Descriptor" entities
  struct DescriptorRangeDesc {
    uint32_t baseRegisterIndex;         // "VKBindingOffsets" not applied to "MUTABLE" and "INPUT_ATTACHMENT" to avoid confusion
    uint32_t descriptorNum;             // treated as max size if "VARIABLE_SIZED_ARRAY" flag is set
    DescriptorType descriptorType;
    StageBits shaderStages;
    DescriptorRangeBits flags;
  };

  // "DescriptorSet" consists of "DescriptorRange" entities
  struct DescriptorSetInfo {
    uint32_t registerSpace;             // must be unique, avoid big gaps
    const DescriptorRangeDesc *ranges;
    uint32_t rangeNum;
    DescriptorSetBits flags;
  };

  // "PipelineLayout" consists of "DescriptorSet" descriptions and root parameters
  struct RootConstantInfo {           // aka push constants block
    uint32_t registerIndex;
    uint32_t size;
    StageBits shaderStages;
  };

  struct RootDescriptorInfo {         // aka push descriptor
    uint32_t registerIndex;
    DescriptorType descriptorType; // a non-typed descriptor type
    StageBits shaderStages;
  };

  // https://learn.microsoft.com/en-us/windows/win32/direct3d12/root-signature-limits#static-samplers
  struct RootSamplerInfo {            // aka static (immutable) sampler
    uint32_t registerIndex;
    SamplerInfo info;
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
  struct PipelineLayoutInfo {
    uint32_t rootRegisterSpace;         // must be unique, avoid big gaps
    const RootConstantInfo *rootConstants;
    uint32_t rootConstantCount;
    const RootDescriptorInfo *rootDescriptors;
    uint32_t rootDescriptorCount;
    const RootSamplerInfo *rootSamplers;
    uint32_t rootSamplerCount;
    const DescriptorSetInfo *descriptorSets;
    uint32_t descriptorSetCount;
    StageBits shaderStages;
    PipelineLayoutBits flags;
  };

  // Descriptor pool
  // https://learn.microsoft.com/en-us/windows/win32/direct3d12/descriptor-heaps
  // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_descriptor_heap_desc
  // https://docs.vulkan.org/refpages/latest/refpages/source/VkDescriptorPoolCreateInfo.html
  struct DescriptorPoolInfo {
    // Maximum number of descriptor sets that can be allocated from this pool
    uint32_t descriptorSetMaxCount;

    // Resource heap
    // - may be directly indexed in shaders via "RESOURCE_HEAP_DIRECTLY_INDEXED" pipeline layout flag
    // - https://docs.vulkan.org/features/latest/features/proposals/VK_EXT_mutable_descriptor_type.html
    uint32_t mutableMaxCount;                 // number of "MUTABLE" descriptors, requires "features.mutableDescriptorType"

    // Sampler heap
    // - may be directly indexed in shaders via "SAMPLER_HEAP_DIRECTLY_INDEXED" pipeline layout flag
    // - root samplers do not count (not allocated from a descriptor pool)
    uint32_t samplerMaxCount;                 // number of "SAMPLER" descriptors

    // Optimized resources (may have various sizes depending on Vulkan implementation)
    uint32_t constantBufferMaxCount;          // number of "CONSTANT_BUFFER" descriptors
    uint32_t textureMaxCount;                 // number of "TEXTURE" descriptors
    uint32_t storageTextureMaxCount;          // number of "STORAGE_TEXTURE" descriptors
    uint32_t bufferMaxCount;                  // number of "BUFFER" descriptors
    uint32_t storageBufferMaxCount;           // number of "STORAGE_BUFFER" descriptors
    uint32_t structuredBufferMaxCount;        // number of "STRUCTURED_BUFFER" descriptors
    uint32_t storageStructuredBufferMaxCount; // number of "STORAGE_STRUCTURED_BUFFER" descriptors
    uint32_t accelerationStructureMaxCount;   // number of "ACCELERATION_STRUCTURE" descriptors, requires "features.rayTracing"
    uint32_t inputAttachmentMaxCount;         // number of "INPUT_ATTACHMENT" descriptors

    DescriptorPoolBits flags;
  };

  // Updating/initializing descriptors in a descriptor set
  struct UpdateDescriptorRangeInfo {
    // Destination
    DescriptorSet *descriptorSet;
    uint32_t rangeIndex;
    uint32_t baseDescriptor;
    // Source & count
    const Descriptor *const* descriptors; // all descriptors must have the same type
    uint32_t descriptorCount;
  };

  // Copying descriptors between descriptor sets
  struct CopyDescriptorRangeInfo {
    // Destination
    DescriptorSet *dstDescriptorSet;
    uint32_t dstRangeIndex;
    uint32_t dstBaseDescriptor;
    // Source & count
    const DescriptorSet *srcDescriptorSet;
    uint32_t srcRangeIndex;
    uint32_t srcBaseDescriptor;
    uint32_t descriptorCount;         // can be "ALL" (source)
  };

  // Binding
  struct SetDescriptorSetInfo {
    uint32_t setIndex;
    const DescriptorSet *descriptorSet;
    BindPoint bindPoint;
  };

  struct SetRootConstantsInfo {   // requires "pipelineLayoutRootConstantMaxSize > 0"
    uint32_t rootConstantIndex;
    const void* data;
    uint32_t size;
    uint32_t offset;                // requires "features.rootConstantsOffset"
    BindPoint bindPoint;
  };

  struct SetRootDescriptorInfo {  // requires "pipelineLayoutRootDescriptorMaxNum > 0"
    uint32_t rootDescriptorIndex;
    Descriptor *descriptor;
    uint32_t offset;                // a non-"CONSTANT_BUFFER" descriptor requires "features.nonConstantBufferRootDescriptorOffset"
    BindPoint bindPoint;
  };
#pragma endregion

//============================================================================================================================================================================================
#pragma region [ Graphics pipeline: input assembly ]
//============================================================================================================================================================================================
  struct D3DVertexAttribute {
    const char* semanticName;
    uint32_t semanticIndex;
  };

  struct VulkanVertexAttribute {
    uint32_t location;
  };

  struct VertexAttributeInfo {
    D3DVertexAttribute d3d;
    VulkanVertexAttribute vulkan;
    uint32_t offset;
    DataFormat format;
    uint16_t streamIndex;
  };

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkVertexInputRate.html
  enum class VertexStreamStepRate: uint8_t {
    PerVertex,
    PerInstance
  };

  struct VertexStreamInfo {
    uint16_t bindingSlot;
    VertexStreamStepRate stepRate;
    uint16_t stride; // fallback if "features.extendedDynamicState" is not supported
  };

  struct VertexInputInfo {
    const VertexAttributeInfo *attributes;
    uint8_t attributeCount;
    const VertexStreamInfo *streams;
    uint8_t streamCount;
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
    PatchList,
    Count
  };

  enum class PrimitiveRestart: uint8_t {
    Disabled,
    Indices_uint16, // index "0xFFFF" enforces primitive restart
    Indices_uint32  // index "0xFFFFFFFF" enforces primitive restart
  };

  struct InputAssemblyInfo {
    Topology topology;
    uint8_t tessControlPointNum;
    PrimitiveRestart primitiveRestart;
  };
#pragma endregion

//============================================================================================================================================================================================
#pragma region [ Graphics pipeline: rasterization ]
//============================================================================================================================================================================================
  // https://docs.vulkan.org/refpages/latest/refpages/source/VkPolygonMode.html
  // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_fill_mode
  enum class FillMode: uint8_t {
    Solid,
    Wireframe,
    Count
  };

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkCullModeFlagBits.html
  // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_cull_mode
  enum class CullMode: uint8_t {
    None,
    Front,
    Back,
    Count
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
  struct DepthBiasInfo {
    float constant;
    float clamp;
    float slope;
  };

  inline bool depthBiasEnabled(const DepthBiasInfo& depthBiasInfo) {
    return depthBiasInfo.constant != 0.0f || depthBiasInfo.slope != 0.0f;
  }

  struct RasterizationInfo {
    DepthBiasInfo depthBias;
    FillMode fillMode;
    CullMode cullMode;
    bool frontCounterClockwise;
    bool depthClamp;
    bool lineSmoothing;         // requires "features.lineSmoothing"
    bool conservativeRaster;    // requires "tiers.conservativeRaster != 0"
    bool shadingRate;           // requires "tiers.shadingRate != 0", expects "CmdSetShadingRate" and optionally "RenderingDesc::shadingRate"
  };

  struct MultisampleInfo {
    uint32_t sampleMask;        // can be "ALL"
    uint8_t sampleCount;
    bool alphaToCoverage;
    bool sampleLocations;       // requires "tiers.sampleLocations != 0", expects "CmdSetSampleLocations"
  };
#pragma endregion

//============================================================================================================================================================================================
#pragma region [ Graphics pipeline: output merger ]
//============================================================================================================================================================================================
  enum class Multiview: uint8_t {
    // Destination "viewport" and/or "layer" must be set in shaders explicitly, "viewMask" for rendering can be < than the one used for pipeline creation (D3D12 style)
    Flexible,       // requires "features.flexibleMultiview"

    // View instances go to statically assigned corresponding attachment layers, "viewMask" for rendering must match the one used for pipeline creation (VK style)
    LayerBased,    // requires "features.layerBasedMultiview"

    // View instances go to statically assigned corresponding viewports, "viewMask" for pipeline creation is unused (D3D11 style)
    ViewportBased  // requires "features.viewportBasedMultiview"
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

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkBlendFactor.html
  // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_blend
  // S0 - source color 0
  // S1 - source color 1
  // D - destination color
  // C - blend constants, set by "CmdSetBlendConstants"
  enum class BlendFactor: uint8_t {
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

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkPipelineDepthStencilStateCreateInfo.html
  struct DepthAttachmentDesc {
    CompareOp compareOp;
    bool write;
    bool boundsTest; // requires "features.depthBoundsTest", expects "CmdSetDepthBounds"
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

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkStencilOpState.html
  // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_depth_stencil_desc
  struct StencilInfo {
    CompareOp compareOp; // "compareOp != NONE", expects "CmdSetStencilReference"
    StencilOp failOp;
    StencilOp passOp;
    StencilOp depthFailOp;
    uint8_t writeMask;
    uint8_t compareMask;
  };

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkPipelineColorBlendAttachmentState.html
  // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_render_target_blend_desc
  struct BlendInfo {
    BlendFactor srcFactor;
    BlendFactor dstFactor;
    BlendOp op;
  };

  struct StencilAttachmentDesc {
    StencilInfo front;
    StencilInfo back; // requires "features.independentFrontAndBackStencilReferenceAndMasks" for "back.writeMask"
  };

  struct ColorAttachmentInfo {
    DataFormat format;
    BlendInfo colorBlend;
    BlendInfo alphaBlend;
    ColorWriteBits colorWriteMask;
    bool blendEnabled;
  };

  struct OutputMergerInfo {
    const ColorAttachmentInfo *colors;
    uint32_t colorCount;
    DepthAttachmentDesc depth;
    StencilAttachmentDesc stencil;
    DataFormat depthStencilFormat;
    LogicOp logicOp;                   // requires "features.logicOp"
    uint32_t viewMask;          // if non-0, requires "viewMaxNum > 1"
    Multiview multiview;   // if "viewMask != 0", requires "features.(xxx)Multiview"
  };
#pragma endregion

//============================================================================================================================================================================================
#pragma region [ Pipelines ]
//============================================================================================================================================================================================
  struct PipelineCacheInfo {
    const void *data; // "data = NULL" means empty cache
    uint64_t size;
  };

  // https://docs.vulkan.org/guide/latest/robustness.html
  enum class Robustness: uint8_t {
    Default, // don't care, follow device settings (VK level when used on a device)
    Off, // no overhead, no robust access (out-of-bounds access is not allowed)
    Vulkan, // minimal overhead, partial robust access
    D3D12 // moderate overhead, D3D12-level robust access (requires "VK_EXT_robustness2", soft fallback to VK mode)
  };

  ENGINE_BITS(GraphicsPipelineBits, uint8_t,
    None                = 0,
    FailOnCacheMiss     = ENGINE_BIT(0) // "CreateGraphicsPipeline" returns "FAILURE" if the pipeline is not found in the supplied cache (requires "features.pipelineCacheControl")
  );

  ENGINE_BITS(ComputePipelineBits, uint8_t,
    None                = 0,
    FailOnCacheMiss     = ENGINE_BIT(0) // "CreateComputePipeline" returns "FAILURE" if the pipeline is not found in the supplied cache (requires "features.pipelineCacheControl")
  );

  // It's recommended to use "NRI.hlsl" in the shader code
  struct ShaderInfo {
    StageBits stage;
    const void* bytecode; // see "features.shaderBytecodeXXX"
    uint64_t size;
    const char* entryPointName;
  };

  struct GraphicsPipelineInfo {
    const PipelineLayout *pipelineLayout;
    const VertexInputInfo *vertexInput;
    InputAssemblyInfo inputAssembly;
    RasterizationInfo rasterization;
    const MultisampleInfo *multisample;
    OutputMergerInfo outputMerger;
    const ShaderInfo *shaders;
    uint32_t shaderCount;
    GraphicsPipelineBits flags;
    Robustness robustness;
    const PipelineCache *cache; // if non-NULL, pipeline creation can be served from a cached blob and the result will be added to the cache on a miss
  };

  struct ComputePipelineInfo {
    const PipelineLayout *pipelineLayout;
    ShaderInfo shader;
    ComputePipelineBits flags;
    Robustness robustness;
    const PipelineCache *cache; // if non-NULL, pipeline creation can be served from a cached blob and the result will be added to the cache on a miss
  };
#pragma endregion

//============================================================================================================================================================================================
#pragma region [ Device and capabilities ]
//============================================================================================================================================================================================
  ENGINE_BITS(GraphicsBackend, uint8_t,
    None    = ENGINE_BIT(0), // Supports everything, does nothing, returns dummy non-NULL objects and ~0-filled descs, available if "NRI_ENABLE_NONE_SUPPORT = ON" in CMake
    D3D11   = ENGINE_BIT(1), // Direct3D 11 (feature set 11.1), available if "NRI_ENABLE_D3D11_SUPPORT = ON" in CMake (https://microsoft.github.io/DirectX-Specs/d3d/archive/D3D11_3_FunctionalSpec.htm)
    D3D12   = ENGINE_BIT(2), // Direct3D 12 (D3D12_SDK_VERSION 4 or 619+), available if "NRI_ENABLE_D3D12_SUPPORT = ON" in CMake (https://microsoft.github.io/DirectX-Specs/)
    Vulkan  = ENGINE_BIT(3) // Vulkan 1.4+, 1.3++ or 1.2+++ (can be used on MacOS via MoltenVK), available if "NRI_ENABLE_VK_SUPPORT = ON" in CMake (https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html)
    // WGPU    = ENGINE_BIT(4)  // WebGPU via wgpu-native, available if "NRI_ENABLE_WGPU_SUPPORT = ON" in CMake (https://github.com/gfx-rs/wgpu-native)
  );

  // TODO: temporary
  enum class Message {
    INFO,
    WARNING,
    ERROR,
    MAX_NUM
  };

  struct CallbackInterface {
    void (*messageCallback)(Message messageType, const char *file, uint32_t line, const char *message, void *userArg);
    void (*abortExecution)(void *userArg); // break on "Message::ERROR" if provided
    void *userArg;
  };

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkQueueFlagBits.html
  // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_command_list_type
  enum class QueueType: uint8_t {
    Graphics,
    Compute,
    Copy,
    Count, // Used to represent the queue count. Not a valid queue type
  };

  struct QueueFamilyInfo {
    const float *queuePriorities; // [-1; 1]: low < 0, normal = 0, high > 0 ("queueNum" entries expected)
    std::uint32_t queueCount;
    QueueType queueType;
  };

  struct QueueFamilyProperties {
    uint32_t queueCount;
    bool graphics;
    bool compute;
    bool copy;
    bool sparse;
    bool videoDecode;
    bool videoEncode;
    bool protect;
    bool opticalFlow;
  };

  ENGINE_FORCE_INLINE QueueType selectSuitableQueueType(
    const QueueFamilyProperties& props,
    std::array<uint32_t, static_cast<size_t>(QueueType::Count)> &scores
  ) {
    { // Prefer as much features as possible
      size_t index = static_cast<size_t>(QueueType::Graphics);
      uint32_t score = ((props.graphics ? 100 : 0) + (props.compute ? 10 : 0) + (props.copy ? 10 : 0) + (props.sparse ? 5 : 0) + (props.videoDecode ? 2 : 0) + (props.videoEncode ? 2 : 0) + (props.protect ? 1 : 0) + (props.opticalFlow ? 1 : 0));

      if (props.graphics && score > scores[index]) {
        scores[index] = score;
        return QueueType::Graphics;
      }
    }

    { // Prefer compute-only
      size_t index = static_cast<size_t>(QueueType::Compute);
      uint32_t score = ((!props.graphics ? 10 : 0) + (props.compute ? 100 : 0) + (!props.copy ? 10 : 0) + (props.sparse ? 5 : 0) + (!props.videoDecode ? 2 : 0) + (!props.videoEncode ? 2 : 0) + (props.protect ? 1 : 0) + (!props.opticalFlow ? 1 : 0));

      if (props.compute && score > scores[index]) {
        scores[index] = score;
        return QueueType::Compute;
      }
    }

    { // Prefer copy-only
      size_t index = static_cast<size_t>(QueueType::Copy);
      uint32_t score = ((!props.graphics ? 10 : 0) + (!props.compute ? 10 : 0) + (props.copy ? 100 * props.queueCount : 0) + (props.sparse ? 5 : 0) + (!props.videoDecode ? 2 : 0) + (!props.videoEncode ? 2 : 0) + (props.protect ? 1 : 0) + (!props.opticalFlow ? 1 : 0));

      if (props.copy && score > scores[index]) {
        scores[index] = score;
        return QueueType::Copy;
      }
    }

    return QueueType::Count;
  }

  struct VulkanExtensions {
    const char *const*instanceExtensions;
    uint32_t instanceExtensionCount;
    const char *const*deviceExtensions;
    uint32_t deviceExtensionCount;
  };

  struct VulkanBindingOffsets {
    uint32_t sRegister; // samplers
    uint32_t tRegister; // shader resources, including acceleration structures (SRVs)
    uint32_t bRegister; // constant buffers
    uint32_t uRegister; // storage shader resources (UAVs)
  };

  enum class Vendor: uint8_t {
    Unknown,
    Nvidia,
    AMD,
    Intel
  };

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkPhysicalDeviceType.html
  enum class DeviceType: uint8_t {
    Unknown,
    Software, // CPU
    Virtual, // remote desktop?
    Integrated, // UMA
    Discrete, // yes, please!
    Count
  };

  struct DeviceUID {
    uint64_t low;
    uint64_t high;
  };

  struct PhysicalDeviceInfo {
    char name[256]{};
    DeviceUID uid{}; // "LUID" (preferred) if "uid.high = 0", or "UUID" otherwise
    uint64_t videoMemorySize{};
    uint64_t sharedSystemMemorySize{};
    uint32_t deviceId{};
    uint32_t driverVersion{}; // GAPI and OS dependent
    std::array<uint32_t, static_cast<uint32_t>(QueueType::Count)> queueCount;
    Vendor vendor = Vendor::Unknown;
    DeviceType deviceType = DeviceType::Unknown;
    GraphicsBackend supportedGraphicsBackends = GraphicsBackend::None;
  };

  struct DeviceCreateInfo {
    GraphicsBackend graphicsBackend = GraphicsBackend::None;
    Robustness robustness{};
    PhysicalDeviceInfo *physicalDeviceInfo{};
    CallbackInterface callbackInterface{};

    // One "GRAPHICS" queue is created by default
    const QueueFamilyInfo *queueFamilies{};
    uint32_t queueFamilyCount{}; // put "GRAPHICS" queue at the beginning of the list

    // D3D specific
    uint32_t d3dShaderExtRegister{};
    // vendor specific shader extensions (default is "NRI_SHADER_EXT_REGISTER", space is always "0")
    uint32_t d3dZeroBufferSize{};
    // no "memset" functionality in D3D, "CmdZeroBuffer" implemented via a bunch of copies (4 Mb by default)

    // Vulkan specific
    VulkanBindingOffsets vulkanBindingOffsets{};
    VulkanExtensions vulkanExtensions{}; // Extensions to enable

    // Switches (disabled by default)
    bool enableNRIValidation{}; // embedded validation layer, checks for NRI specifics
    bool enableGraphicsAPIValidation{}; // GAPI-provided validation layer
    bool enableD3D11CommandBufferEmulation{}; // enable? but why? (auto-enabled if deferred contexts are not supported)
    bool enableD3D12RayTracingValidation{};
    // slow but useful, can only be enabled if envvar "NV_ALLOW_RAYTRACING_VALIDATION" is set to "1"
    bool enableMemoryZeroInitialization{}; // page-clears are fast, but memory is not cleared by default in VK

    // Switches (enabled by default)
    bool disableVKRayTracing{}; // to save CPU memory in some implementations
    bool disableD3D12EnhancedBarriers{};
    // even if AgilitySDK is in use, some apps still use legacy barriers. It can be important for integrations
  };

  // Feature support coverage: https://vulkan.gpuinfo.org/ and https://d3d12infodb.boolka.dev/
  struct DeviceInfo {
    // Common
    PhysicalDeviceInfo physicalDeviceInfo; // "queueCount" reflects available number of queues per "QueueType"
    GraphicsBackend graphicsBackend = GraphicsBackend::None;
    uint16_t nriVersion{};
    uint16_t shaderModel{}; // see "NriShaderModel"

    // Viewport
    struct {
      uint32_t maxNum;
      int32_t boundsMin;
      int32_t boundsMax;
    } viewport{};

    // Dimensions
    struct {
      uint32_t typedBufferMaxDim;
      uint16_t attachmentMaxDim;
      uint16_t attachmentLayerMaxNum;
      uint16_t texture1DMaxDim;
      uint16_t texture2DMaxDim;
      uint16_t texture3DMaxDim;
      uint16_t textureLayerMaxNum;
    } dimensions{};

    // Precision bits
    struct {
      uint32_t viewportBits;
      uint32_t subPixelBits;
      uint32_t subTexelBits;
      uint32_t mipmapBits;
    } precision{};

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
    } memory{};

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
    } memoryAlignment{};

    // Pipeline layout (see "FitPipelineLayoutSettingsIntoDeviceLimits")
    // D3D12 only: "rootConstantSize" + "descriptorSetNum" * 4 + "rootDescriptorNum" * 8 + "reservedSize" <= 256, where
    // "reservedSize" is 8 bytes for "ENABLE_DRAW_PARAMETERS_EMULATION" and 4 bytes for "ENABLE_DRAW_INDEX_EMULATION"
    struct {
      uint32_t descriptorSetMaxNum;
      uint32_t rootConstantMaxSize;
      uint32_t rootDescriptorMaxNum;
    } pipelineLayout{};

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
    } descriptorSet{};

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
    } shaderStage{};

    // Acceleration structure
    struct {
      uint64_t primitiveMaxNum; // per BLAS
      uint64_t geometryMaxNum; // per BLAS
      uint64_t instanceMaxNum; // per TLAS
      uint32_t micromapSubdivisionMaxLevel;
    } accelerationStructure{};

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
    } wave{};

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
    } other{};

    // Tiers (0 - unsupported)
    struct {
      // https://microsoft.github.io/DirectX-Specs/d3d/ConservativeRasterization.html#tiered-support
      // 1 - 1/2 pixel uncertainty region and does not support post-snap degenerates
      // 2 - reduces the maximum uncertainty region to 1/256 and requires post-snap degenerates not be culled
      // 3 - maintains a maximum 1/256 uncertainty region and adds support for inner input coverage, aka "SV_InnerCoverage"
      uint8_t conservativeRaster;

      // https://microsoft.github.io/DirectX-Specs/d3d/ProgrammableSamplePositions.html#hardware-tiers
      // 1 - a single sample pattern can be specified to repeat for every pixel ("locationNum / sampleCount" ratio must be 1 in "CmdSetSampleLocations"),
      //     1x and 16x sample counts do not support programmable locations
      // 2 - four separate sample patterns can be specified for each pixel in a 2x2 grid ("locationNum / sampleCount" ratio can be 1 or 4 in "CmdSetSampleLocations"),
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
      // 1 - only "CONSTANT_BUFFER" and "STORAGE" descriptors in range must be valid
      // 2 - only referenced descriptors must be valid
      uint8_t resourceBinding;

      // 1 - unbound arrays with dynamic indexing
      // 2 - D3D12 dynamic resources: https://microsoft.github.io/DirectX-Specs/d3d/HLSL_SM_6_6_DynamicResources.html
      uint8_t bindless;

      // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_resource_heap_tier
      // 1 - a "Memory" can support resources from all 3 categories: buffers, attachments, all other textures
      uint8_t memory;
    } tiers{};

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
      bool shaderBytecodeDXBC; // DXBC can be passed to "ShaderInfo::bytecode"
      bool shaderBytecodeDXIL; // DXIL can be passed to "ShaderInfo::bytecode"
      bool shaderBytecodeSPIRV; // SPIRV can be passed to "ShaderInfo::bytecode", WGPU expects Vulkan 1.2 environment
      bool shaderBytecodeWGSL; // WGSL can be passed to "ShaderInfo::bytecode"

      // Queries
      bool occlusion; // see "QueryType::OCCLUSION"
      bool timestamp; // see "QueryType::TIMESTAMP"
      bool timestampCopyQueue; // see "QueryType::TIMESTAMP_COPY_QUEUE"
      bool calibratedTimestamps; // see "GetCalibratedTimestamps"

      // Shading rate
      bool additionalShadingRates; // see "ShadingRate"
      bool sumShadingRateCombiner; // see "ShadingRateCombiner::SUM"

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
      bool componentSwizzle; // see "ComponentSwizzle" (unsupported only in D3D11)
      bool independentFrontAndBackStencilReferenceAndMasks; // see "StencilAttachmentDesc::back"
      bool filterOpMinMax; // see "FilterOp"
      bool logicOp; // see "LogicOp"
      bool depthBoundsTest; // see "DepthAttachmentDesc::boundsTest"
      bool drawIndirectCount; // see "countBuffer" and "countBufferOffset"
      bool lineSmoothing; // see "RasterizationInfo::lineSmoothing"
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
    } features{};

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
    } shaderFeatures{};
  };

  // Memory requirements for a resource (buffer or texture)
  struct MemoryInfo {
    uint64_t size;
    uint32_t alignment;
    uint32_t type; // Contains some encoded implementation specific details
    bool mustBeDedicated; // must be put into a dedicated "Memory" object, containing only 1 object with offset = 0
  };

  // NRI tries to ease your life and avoid using "queue ownership transfers" (see "TextureBarrierDesc").
  // In most of cases "SharingMode" can be ignored. Where is it needed?
  // - VK: use "EXCLUSIVE" for attachments participating into multi-queue activities to preserve DCC (Delta Color Compression) on some HW
  // - D3D12: use "SIMULTANEOUS" to concurrently use a texture as a "SHADER_RESOURCE" (or "SHADER_RESOURCE_STORAGE") and as a "COPY_DESTINATION" for non overlapping texture regions
  // https://docs.vulkan.org/refpages/latest/refpages/source/VkSharingMode.html
  enum class SharingMode: uint8_t {
    Concurrent,     // VK: lazy default to avoid dealing with "queue ownership transfers", auto-optimized to "EXCLUSIVE" if all queues have the same type
    Exclusive,      // VK: may be used for attachments to preserve DCC on some HW in the cost of making a "queue ownership transfer"

    // https://microsoft.github.io/DirectX-Specs/d3d/D3D12EnhancedBarriers.html#single-queue-simultaneous-access
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_resource_flags
    Simultaneous    // D3D12: strengthened variant of "CONCURRENT", allowing simultaneous multiple readers and one writer for a texture (requires "Layout::GENERAL")
  };

  enum class MemoryLocation: uint8_t {
    Device,
    DeviceUpload, // soft fallback to "HOST_UPLOAD" if "deviceUploadHeapSize = 0"
    HostUpload,
    HostReadback
  };

  inline DeviceUID createDeviceUID(uint8_t luid[8], uint8_t uuid[16], bool isLuidValid) {
    DeviceUID out = {};

    if (isLuidValid)
      memcpy(&out.low, luid, sizeof(out.low));
    else {
      memcpy(&out.low, uuid, sizeof(out.low));
      memcpy(&out.high, uuid + 8, sizeof(out.high));
    }

    return out;
  }

  inline bool compareDeviceUID(const DeviceUID& a, const DeviceUID& b) {
    return a.low == b.low && a.high == b.high;
  }

  inline Vendor getVendorFromID(uint32_t vendorID) {
    switch (vendorID) {
      case 0x10DE:
        return Vendor::Nvidia;
      case 0x1002:
        return Vendor::AMD;
      case 0x8086:
        return Vendor::Intel;
      default: break;
    }

    return Vendor::Unknown;
  }

  struct CoreInterface;
  class Device {
    public:
      Device(const CallbackInterface &callbacks): callbackInterface(callbacks) {
      }

      virtual ~Device() = default;

      CallbackInterface callbackInterface;

    private:
      virtual Result loadInterface(Device &device, CoreInterface &coreInterface) { return Result::Unsupported; }
      virtual Result loadInterface(Device &device, SwapChainInterface &coreInterface) { return Result::Unsupported; }

      friend Result getInterface(Device &device, CoreInterface &coreInterface);
      friend Result getInterface(Device &device, SwapChainInterface &coreInterface);
  };

  Result getPhysicalDevices();

  Result createDevice(const DeviceCreateInfo &createInfo, Device *&device);

  void destroyDevice(Device *device);
#pragma endregion

//============================================================================================================================================================================================
#pragma region [ Resources ]
//============================================================================================================================================================================================
  // https://docs.vulkan.org/refpages/latest/refpages/source/VkImageType.html
  // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_resource_dimension
  enum class TextureDimension: uint8_t {
    Dimension1D,
    Dimension2D,
    Dimension3D
  };

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkImageUsageFlagBits.html
  // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_resource_flags
  ENGINE_BITS(TextureUsageBits, uint8_t,             // Min compatible access:                   Usage:
    None                                = 0,
    ShaderResource                      = ENGINE_BIT(0),    // ShaderResource                           Read-only shader resource view (SRV)
    ShaderResourceStorage               = ENGINE_BIT(1),    // ShaderResourceStorage                    Read/write shader resource view (UAV)
    ColorAttachment                     = ENGINE_BIT(2),    // ColorAttachment                          Color attachment (render target)
    DepthStencilAttachment              = ENGINE_BIT(3),    // DepthStencilAttachmentRead/Write         Depth-stencil attachment (depth-stencil target)
    ShadingRateAttachment               = ENGINE_BIT(4),    // ShadingRateAttachment                    Shading rate attachment (source)
    InputAttachment                     = ENGINE_BIT(5)     // InputAttachment                          Subpass input (read on-chip tile cache)
  );

  struct TextureInfo {
    TextureDimension dimension;
    TextureUsageBits usage;
    DataFormat format;
    uint16_t width;
    uint16_t height;
    uint16_t depth;
    uint16_t mipCount;
    uint16_t layerCount;
    uint8_t sampleCount;
    SharingMode sharingMode;
    ClearValue optimizedClearValue;    // D3D12: not needed on desktop, since any HW can track many clear values
  };
#pragma endregion
  /* clang-format on */

  struct CoreInterface {
    // Get
    const DeviceInfo& (*getDeviceInfo)(const Device &device);
    // const NriRef(BufferDesc)    (NRI_CALL *GetBufferDesc)           (const NriRef(Buffer) buffer);
    // const NriRef(TextureDesc)   (NRI_CALL *GetTextureDesc)          (const NriRef(Texture) texture);
    // Nri(FormatSupportBits)      (NRI_CALL *GetFormatSupport)        (const NriRef(Device) device, Nri(Format) format);

    // Returns one of the pre-created queues (see "DeviceCreationDesc" or wrapper extensions)
    // Return codes: "UNSUPPORTED" (no queues of "queueType") or "INVALID_ARGUMENT" (if "queueIndex" is out of bounds).
    // Getting "COMPUTE" and/or "COPY" queues switches VK sharing mode to "VK_SHARING_MODE_CONCURRENT" for resources created without "queueExclusive" flag.
    // This approach is used to minimize number of "queue ownership transfers", but also adds a requirement to "get" all async queues BEFORE creation of
    // resources participating into multi-queue activities. Explicit use of "queueExclusive" removes any restrictions.
    Result (*getQueue)(Device &device, QueueType queueType, uint32_t queueIndex, Queue *&queue);

    // Create (doesn't assume allocation of big chunks of memory on the device, but it happens for some entities implicitly)
    // Result (*createCommandAllocator) (Queue &queue, NriOut NriRef(CommandAllocator*) commandAllocator);
    // Result (*createCommandBuffer) (CommandAllocator &commandAllocator, NriOut NriRef(CommandBuffer*) commandBuffer);
    // Result (*createFence) (Device &device, uint64_t initialValue, NriOut NriRef(Fence*) fence);
    // Result (*createDescriptorPool) (Device &device, const NriRef(DescriptorPoolDesc) descriptorPoolDesc, NriOut NriRef(DescriptorPool*) descriptorPool);
    Result (*createPipelineLayout)(Device &device, const PipelineLayoutInfo &pipelineLayoutInfo, PipelineLayout *&pipelineLayout);
    Result (*createGraphicsPipeline)(Device &device, const GraphicsPipelineInfo &graphicsPipelineDesc, Pipeline *&pipeline);
    // Result (*createComputePipeline) (Device &device, const NriRef(ComputePipelineDesc) computePipelineDesc, NriOut NriRef(Pipeline*) pipeline);
    // Result (*createPipelineCache) (Device &device, const NriRef(PipelineCacheDesc) pipelineCacheDesc, NriOut NriRef(PipelineCache*) pipelineCache); // "OUT_OF_DATE" is returned on stale data, try to start over with an empty cache
    // Result (*createQueryPool) (Device &device, const NriRef(QueryPoolDesc) queryPoolDesc, NriOut NriRef(QueryPool*) queryPool);
    // Result (*createSampler) (Device &device, const NriRef(SamplerDesc) samplerDesc, NriOut NriRef(Descriptor*) sampler);
    // Result (*createBufferView) (const BufferViewDesc &bufferViewDesc, NriOut NriRef(Descriptor*) bufferView);
    // Result (*createTextureView) (const TextureViewDesc &textureViewDesc, NriOut NriRef(Descriptor*) textureView);

    // Destroy
    // void                (NRI_CALL *DestroyCommandAllocator)         (NriPtr(CommandAllocator) commandAllocator);
    // void                (NRI_CALL *DestroyCommandBuffer)            (NriPtr(CommandBuffer) commandBuffer);
    // void                (NRI_CALL *DestroyDescriptorPool)           (NriPtr(DescriptorPool) descriptorPool);
    // void                (NRI_CALL *DestroyBuffer)                   (NriPtr(Buffer) buffer);
    // void                (NRI_CALL *DestroyTexture)                  (NriPtr(Texture) texture);
    // void                (NRI_CALL *DestroyDescriptor)               (NriPtr(Descriptor) descriptor);
    void (*destroyPipelineLayout)(PipelineLayout *pipelineLayout);
    void (*destroyPipeline)(Pipeline *pipeline);
    void (*destroyPipelineCache)(PipelineCache *pipelineCache);
    // void                (NRI_CALL *DestroyQueryPool)                (NriPtr(QueryPool) queryPool);
    // void                (NRI_CALL *DestroyFence)                    (NriPtr(Fence) fence);

    // Work submission and synchronization
    // Nri(Result)         (NRI_CALL *QueueSubmit)                     (NriRef(Queue) queue, const NriRef(QueueSubmitDesc) queueSubmitDesc); // to device
    // Nri(Result)         (NRI_CALL *QueueWaitIdle)                   (NriPtr(Queue) queue);
    Result (*deviceWaitIdle)(Device *device);
    // void                (NRI_CALL *Wait)                            (NriRef(Fence) fence, uint64_t value); // on host
    // uint64_t            (NRI_CALL *GetFenceValue)                   (NriRef(Fence) fence);
  };

  Result getInterface(Device &device, CoreInterface &coreInterface);
  Result getInterface(Device &device, SwapChainInterface &coreInterface);
}
