#pragma once
#include <core/rhi/rhi.hpp>
#include <volk.h>

namespace Core::RHI {
  class VulkanDevice;

  struct PushConstantBindingInfo {
    VkShaderStageFlags stages;
    uint32_t offset;
  };

  struct BindingInfo {
    std::vector<DescriptorRangeDesc> ranges;
    std::vector<DescriptorSetInfo> sets;
    std::vector<PushConstantBindingInfo> pushConstants;
    std::vector<uint32_t> pushDescriptors;
    uint32_t rootRegisterSpace{};
    uint32_t rootSamplerBindingOffset{};
  };

  class VulkanPipelineLayout {
    public:
      inline VulkanPipelineLayout(VulkanDevice &device): device(device) {
      }

      inline operator VkPipelineLayout() const {
        return pipelineLayout;
      }

      inline VulkanDevice &getDevice() const {
        return device;
      }

      inline const BindingInfo &getBindingInfo() const {
        return bindingInfo;
      }

      inline VkDescriptorSetLayout getDescriptorSetLayout(uint32_t setIndex) const {
        return descriptorSetLayouts[setIndex];
      }

      ~VulkanPipelineLayout();

      Result create(const PipelineLayoutInfo &pipelineLayoutDesc);

    private:
      void createSetLayout(
        VkDescriptorSetLayout *setLayout,
        const DescriptorSetInfo &descriptorSetDesc,
        const RootSamplerInfo *rootSamplers,
        uint32_t rootSamplerNum,
        bool ignoreGlobalSPIRVOffsets,
        bool isPush
      );

      VulkanDevice &device;
      VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
      BindingInfo bindingInfo;
      std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
      std::vector<VkSampler> immutableSamplers;
  };
}
