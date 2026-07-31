#pragma once
#include <filesystem>
#include <string>
#include <core/memory.hpp>
#include "volk.h"
#include "vulkan_device.hpp"

namespace Core::Graphics {
  class VulkanShader: public RefCounted {
    public:
      VulkanShader(RefCountedPtr<VulkanDevice> device, const std::string &filePath);

      ~VulkanShader();

      const std::vector<VkPipelineShaderStageCreateInfo> &getShaderStages() const { return shaderStages; }

    private:
      RefCountedPtr<VulkanDevice> device;
      VkShaderModule shaderModule {};

      std::filesystem::path filePath;
      std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
  };
}