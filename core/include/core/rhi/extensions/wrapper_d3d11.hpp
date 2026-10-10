// © 2021 NVIDIA Corporation

// Goal: wrapping native D3D11 objects into NRI objects

#pragma once

#define NRI_WRAPPER_D3D11_H 1

#include "device_creation.hpp"

typedef int32_t DXGIFormat;

struct AGSContext;
struct ID3D11Device;
struct ID3D11Resource;
struct ID3D11DeviceContext;

namespace Core::RHI {
  struct DeviceCreationD3D11Desc {
    ID3D11Device* d3d11Device;
    RHI_OPTIONAL AGSContext* agsContext;
    RHI_OPTIONAL CallbackInterface callbackInterface;
    RHI_OPTIONAL AllocationCallbacks allocationCallbacks;
    RHI_OPTIONAL uint32_t d3dShaderExtRegister;  // vendor specific shader extensions (default is "NRI_SHADER_EXT_REGISTER", space is always "0")
    RHI_OPTIONAL uint32_t d3dZeroBufferSize;     // no "memset" functionality in D3D, "CmdZeroBuffer" implemented via a bunch of copies (4 Mb by default)

    // Switches (disabled by default)
    bool enableNRIValidation;                   // embedded validation layer, checks for NRI specifics
    bool enableD3D11CommandBufferEmulation;     // enable? but why? (auto-enabled if deferred contexts are not supported)

    // Switches (enabled by default)
    bool disableNVAPIInitialization;            // at least NVAPI requires calling "NvAPI_Initialize" in DLL/EXE where the device is created
  };

  struct CommandBufferD3D11Desc {
    ID3D11DeviceContext* d3d11DeviceContext;
  };

  struct BufferD3D11Desc {
    ID3D11Resource* d3d11Resource;
    RHI_OPTIONAL const BufferDesc* desc;  // not all information can be retrieved from the resource if not provided
  };

  struct TextureD3D11Desc {
    ID3D11Resource* d3d11Resource;
    RHI_OPTIONAL DXGIFormat format;             // must be provided "as a compatible typed format" if the resource is typeless
  };

  // Threadsafe: yes
  struct WrapperD3D11Interface {
    Result (*CreateCommandBufferD3D11)    (Device &Device, const CommandBufferD3D11Desc &CommandBufferD3D11Desc, RHI_OUT CommandBuffer *&commandBuffer);
    Result (*CreateBufferD3D11)           (Device &Device, const BufferD3D11Desc &BufferD3D11Desc, RHI_OUT Buffer *&buffer);
    Result (*CreateTextureD3D11)          (Device &Device, const TextureD3D11Desc &TextureD3D11Desc, RHI_OUT Texture *&texture);
  };

  Result nriCreateDeviceFromD3D11Device(const DeviceCreationD3D11Desc &deviceDesc, RHI_OUT Device *&device);
}