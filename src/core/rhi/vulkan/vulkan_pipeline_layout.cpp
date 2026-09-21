#include "vulkan_device.hpp"
#include "vulkan_pipeline_layout.hpp"

namespace Core::RHI {
  VulkanPipelineLayout::~VulkanPipelineLayout() {
    const auto allocationCallbacks = device.getAllocationCallbacks();

    if (pipelineLayout)
      vkDestroyPipelineLayout(device, pipelineLayout, allocationCallbacks);

    for (auto handle: descriptorSetLayouts)
      vkDestroyDescriptorSetLayout(device, handle, allocationCallbacks);

    for (auto handle: immutableSamplers)
      vkDestroySampler(device, handle, allocationCallbacks);
  }

  void VulkanPipelineLayout::createSetLayout(VkDescriptorSetLayout *setLayout, const DescriptorSetInfo &descriptorSetDesc, const RootSamplerInfo *rootSamplers, uint32_t rootSamplerNum, bool ignoreGlobalSPIRVOffsets, bool isPush) {

  }

  Result VulkanPipelineLayout::create(const PipelineLayoutInfo &pipelineLayoutDesc) {
    return Result::Success;
  }
}
