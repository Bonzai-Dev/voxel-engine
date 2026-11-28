#include <stdexcept>
#include <string>
#include <vector>
#include <renderer/renderer_context.hpp>
#include <Extensions/NRISwapChain.h>
#include <Extensions/NRIDeviceCreation.h>
// #include "slang-com-ptr.h"

namespace {
  using namespace Engine;

  // inline Slang::ComPtr<slang::IGlobalSession> slangGlobalSession;

  constexpr nri::VKBindingOffsets VK_BINDING_OFFSETS = {0, 128, 32, 64};

  void checkNri(nri::Result result) {
    if (result != nri::Result::SUCCESS) {
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
    checkNri(nri::nriEnumerateAdapters(nullptr, adapterCount));

    std::vector<nri::AdapterDesc> adapters(adapterCount);
    checkNri(nri::nriEnumerateAdapters(adapters.data(), adapterCount));

    const nri::AdapterDesc *selectedAdapter = nullptr;
    for (const auto &adapter: adapters) {
      if (adapter.supportedGraphicsAPIs & nri::GraphicsAPI::VK) {
        selectedAdapter = &adapter;
        break;
      }
    }

    if (!selectedAdapter)
      throw std::runtime_error("No adapter supports Vulkan");

    nri::DeviceCreationDesc deviceDesc = {};
    deviceDesc.graphicsAPI = nri::GraphicsAPI::VK;
    deviceDesc.adapterDesc = selectedAdapter;
    deviceDesc.vkBindingOffsets = VK_BINDING_OFFSETS;
#ifdef ENGINE_DEBUG
    deviceDesc.enableNRIValidation = true;
    deviceDesc.enableGraphicsAPIValidation = true;
#endif
    checkNri(nri::nriCreateDevice(deviceDesc, renderingDevice));

    try {
      checkNri(nri::nriGetInterface(
        *renderingDevice,
        NRI_INTERFACE(nri::CoreInterface),
        static_cast<nri::CoreInterface*>(this)
      ));

      checkNri(nri::nriGetInterface(
        *renderingDevice,
        NRI_INTERFACE(nri::SwapChainInterface),
        static_cast<nri::SwapChainInterface*>(this)
      ));
    } catch (...) {
      nri::nriDestroyDevice(renderingDevice);
      throw;
    }
  }

  RendererContext::~RendererContext() {
    if (!renderingDevice)
      return;

    DeviceWaitIdle(renderingDevice);
    nri::nriDestroyDevice(renderingDevice);
  }
}
