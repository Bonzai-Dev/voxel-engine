#include <vector>
#include <string>
#include <core/io/file.hpp>
#include "volk.h"
#include "vulkan_rendering_device.hpp"
#include "vulkan_shader.hpp"

namespace Core::Graphics {
  VulkanShader::VulkanShader(RefCountedPtr<VulkanDevice> device, const std::string &filePath) : filePath(filePath), device(device) {
    std::vector<char> spirv = readBinaryFile(this->filePath.relative_path().string());

    VkShaderModuleCreateInfo shaderModuleCreateInfo {};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize = spirv.size() * sizeof(char);
    shaderModuleCreateInfo.pCode = reinterpret_cast<std::uint32_t*>(spirv.data());

    VULKAN_CHECK(vkCreateShaderModule(device->logicalDevice, &shaderModuleCreateInfo, nullptr, &shaderModule));

    shaderStages.push_back({
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_VERTEX_BIT,
      .module = shaderModule,
      .pName = "main"
    });

    shaderStages.push_back({
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
      .module = shaderModule,
      .pName = "main"
    });
  }

  VulkanShader::~VulkanShader() {
    vkDestroyShaderModule(device->logicalDevice, shaderModule, nullptr);
  }
}
