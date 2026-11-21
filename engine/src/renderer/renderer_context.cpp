#include <stdexcept>
#include <string>
#include <vector>
#include <engine/runtime/renderer/renderer_context.hpp>
#include <Extensions/NRISwapChain.h>
#include <Extensions/NRIDeviceCreation.h>
// #include "slang-com-ptr.h"

namespace {
  using namespace Engine;

  // inline Slang::ComPtr<slang::IGlobalSession> slangGlobalSession;

  constexpr RHI::VKBindingOffsets VK_BINDING_OFFSETS = {0, 128, 32, 64};

  void checkNri(RHI::Result result) {
    if (result != RHI::Result::SUCCESS) {
      throw std::runtime_error(
        std::to_string(static_cast<int>(result))
      );
    }
  }

}

namespace Engine::Renderer {
  RendererContext::RendererContext() {
    // SlangGlobalSessionDesc desc = {};
    // slang::createGlobalSession(&desc, slangGlobalSession.writeRef());

    uint32_t adapterCount = 0;
    checkNri(RHI::nriEnumerateAdapters(nullptr, adapterCount));

    std::vector<RHI::AdapterDesc> adapters(adapterCount);
    checkNri(RHI::nriEnumerateAdapters(adapters.data(), adapterCount));

    const RHI::AdapterDesc *selectedAdapter = nullptr;
    for (const auto &adapter: adapters) {
      if (adapter.supportedGraphicsAPIs & RHI::GraphicsAPI::VK) {
        selectedAdapter = &adapter;
        break;
      }

    }
    if (!selectedAdapter)
      throw std::runtime_error("No adapter supports Vulkan");

    RHI::DeviceCreationDesc deviceDesc = {};
    deviceDesc.graphicsAPI = RHI::GraphicsAPI::VK;
    deviceDesc.adapterDesc = selectedAdapter;
    deviceDesc.vkBindingOffsets = VK_BINDING_OFFSETS;
#ifdef ENGINE_DEBUG
    deviceDesc.enableNRIValidation = true;
    deviceDesc.enableGraphicsAPIValidation = true;
#endif
    checkNri(RHI::nriCreateDevice(deviceDesc, renderingDevice));

    try {
      checkNri(RHI::nriGetInterface(
        *renderingDevice,
        NRI_INTERFACE(Engine::RHI::CoreInterface),
        static_cast<RHI::CoreInterface*>(this)
      ));

      checkNri(RHI::nriGetInterface(
        *renderingDevice,
        NRI_INTERFACE(Engine::RHI::SwapChainInterface),
        static_cast<RHI::SwapChainInterface*>(this)
      ));
    } catch (...) {
      RHI::nriDestroyDevice(renderingDevice);
      throw;
    }
  }

  RendererContext::~RendererContext() {
    if (!renderingDevice)
      return;

    DeviceWaitIdle(renderingDevice);
    RHI::nriDestroyDevice(renderingDevice);
  }
}
