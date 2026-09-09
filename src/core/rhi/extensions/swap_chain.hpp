#pragma once
#include <cstdint>
#include <SDL3/SDL_video.h>
#include <core/core.hpp>

namespace Core::RHI {
  class Queue;

  // Color space:
  //  - BT.709 - LDR https://en.wikipedia.org/wiki/Rec._709
  //  - BT.2020 - HDR https://en.wikipedia.org/wiki/Rec._2020
  //  - https://registry.khronos.org/DataFormat/specs/1.4/dataformat.1.4.html#PRIMARY_CONVERSION
  // Transfer function:
  //  - G10 - linear (gamma 1.0)
  //  - G22 - sRGB (gamma ~2.2)
  //  - G2084 - SMPTE ST.2084 (Perceptual Quantization)
  //  - https://registry.khronos.org/DataFormat/specs/1.4/dataformat.1.4.html#TRANSFER_CONVERSION
  // Bits per channel:
  //  - 8, 10, 16 (float)
  enum class SwapChainFormat: uint8_t {
    BT709_G10_16BIT,
    BT709_G22_8BIT,
    BT709_G22_10BIT,
    BT2020_G2084_10BIT
  };

  inline const char *swapChainFormatToString(SwapChainFormat format) {
    switch (format) {
      case SwapChainFormat::BT709_G10_16BIT: return "BT709_G10_16BIT";
      case SwapChainFormat::BT709_G22_8BIT: return "BT709_G22_8BIT";
      case SwapChainFormat::BT709_G22_10BIT: return "BT709_G22_10BIT";
      case SwapChainFormat::BT2020_G2084_10BIT: return "BT2020_G2084_10BIT";
      default: return "Unknown";
    }
  }

  ENGINE_ENUM_BITS(SwapChainBits, uint8_t,
    None                = 0,
    VSync               = ENGINE_BIT(0), // cap framerate to the monitor refresh rate
    Waitable            = ENGINE_BIT(1), // unlock "WaitForPresent" reducing latency (requires "features.waitableSwapChain")
    AllowTearing        = ENGINE_BIT(2), // allow screen tearing if possible
    AllowLowLatency     = ENGINE_BIT(3)  // allow "NRILowLatency" functionality (requires "features.lowLatency")
  );

  // https://registry.khronos.org/vulkan/specs/latest/man/html/VkPresentScalingFlagBitsKHR.html
  enum class Scaling: uint8_t {
    OneToOne,                        // no scaling, 1:1 pixel mapping
    Stretch                          // minified or magnified stretching
  };

  // https://registry.khronos.org/vulkan/specs/latest/man/html/VkPresentGravityFlagBitsKHR.html
  enum class Gravity: uint8_t {
    Min,                             // pixels will gravitate towards the top or left side of the surface (Windows style)
    Max,                             // VK: pixels will gravitate towards the bottom or right side of the surface
    Centered                         // VK: pixels will be centered in the surface
  };

  // SwapChain textures will be created as "color attachment" resources
  // queuedFrameNum = 0 - auto-selection between 1 (for waitable) or 2 (otherwise)
  // queuedFrameNum = 2 - recommended if the GPU frame time is less than the desired frame time, but the sum of 2 frames is greater
  struct SwapChainInfo {
    SDL_Window *window = nullptr;
    const Queue *presentQueue = nullptr;                  // GRAPHICS or COMPUTE (requires "features.presentFromCompute")
    uint16_t width{};
    uint16_t height{};
    uint8_t textureNum{};                         // desired value, real value must be queried using "GetSwapChainTextures"
    SwapChainFormat format{};                // desired format, real value must be queried using "GetTextureDesc" for one of the swap chain textures
    SwapChainBits flags{};
    uint8_t queuedFrameNum{};         // aka "max frame latency", aka "number of frames in flight" (mostly for D3D11)

    // Present scaling and positioning, silently ignored if "features.resizableSwapChain" is not supported or not supported by the implicitly chosen present mode
    Scaling scaling{};           // VK: if scaling is not supported, "OUT_OF_DATE" error is triggered on resizing
    Gravity gravityX{};
    Gravity gravityY{};
  };

  class SwapChain {
  };
}

/*
Typical usage example, valid if the number of swap chain images >= queued frames:

// Creation:
    // Create swap chain
        SwapChainDesc swapChainDesc = {};
        swapChainDesc.textureNum = QUEUED_FRAME_NUM + 1; // assuming no correction after swap chain creation for simplicity
        ...

    // Create in-flight data
        struct SwapChainTexture {
            Fence* acquireSemaphore;
            Fence* releaseSemaphore;
            ...
        } swapChainTextures[16] = {};

        Fence* frameFence;
        NRI.CreateFence(device, 0, frameFence);

        // Use "GetSwapChainTextures" to populate
        for (uint32_t i = 0; i < swapChainDesc.textureNum; i++) {
            NRI.CreateFence(device, SWAPCHAIN_SEMAPHORE, swapChainTextures[i].swapChainAcquireSemaphore);
            NRI.CreateFence(device, SWAPCHAIN_SEMAPHORE, swapChainTextures[i].swapChainReleaseSemaphore);
            ...
        }

// Rendering:
    // Wait for the tail of enqueued frames
        NRI.Wait(frameFence, frameIndex >= QUEUED_FRAME_NUM ? 1 + frameIndex - QUEUED_FRAME_NUM : 0);

    // Acquire a swap chain texture
        uint32_t recycledSemaphoreIndex = frameIndex % swapChainDesc.textureNum;
        Fence* acquireSemaphore = swapChainTextures[recycledSemaphoreIndex].acquireSemaphore;

        uint32_t currentSwapChainTextureIndex = 0;
        NRI.AcquireNextTexture(swapChain, *acquireSemaphore, currentSwapChainTextureIndex);

    // Record command buffers
        ... // use swap chain texture with index "currentSwapChainTextureIndex"

    // Submit
        Fence* releaseSemaphore = swapChainTextures[currentSwapChainTextureIndex].releaseSemaphore;

        FenceSubmitDesc frameFence = {};
        frameFence.fence = m_FrameFence;
        frameFence.value = 1 + frameIndex;

        FenceSubmitDesc textureAcquiredFence = {};
        textureAcquiredFence.fence = acquireSemaphore;
        textureAcquiredFence.stages = COLOR_ATTACHMENT; // COPY, ALL

        FenceSubmitDesc renderingFinishedFence = {};
        renderingFinishedFence.fence = releaseSemaphore;

        FenceSubmitDesc signalFences[] = {renderingFinishedFence, frameFence};

        QueueSubmitDesc queueSubmitDesc = {};
        queueSubmitDesc.waitFences = &textureAcquiredFence;
        queueSubmitDesc.waitFenceNum = 1;
        queueSubmitDesc.commandBuffers = ...;
        queueSubmitDesc.commandBufferNum = ...;
        queueSubmitDesc.signalFences = signalFences;
        queueSubmitDesc.signalFenceNum = 2;

        NRI.QueueSubmit(queue, queueSubmitDesc);

    // Present
        NRI.QueuePresent(swapChain, *releaseSemaphore);
*/

