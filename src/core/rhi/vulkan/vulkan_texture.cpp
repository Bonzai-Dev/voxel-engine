#include "vulkan_backend.hpp"

namespace Core::RHI {
  VulkanTexture::VulkanTexture(VulkanDevice &device):
    device(device) {
  }

  VulkanTexture::~VulkanTexture() {
    if (vmaAllocation)
      vmaDestroyImage(device.getVma(), image, vmaAllocation);
    else
      vkDestroyImage(device, image, &device.vulkanAllocationCallbacks);
  }

  Result VulkanTexture::create(const TextureInfo &textureInfo) {
    info.height = std::max(textureInfo.height, static_cast<uint16_t>(1));
    info.depth = std::max(textureInfo.depth, static_cast<uint16_t>(1));
    info.mipNum = std::max(textureInfo.mipNum, static_cast<uint16_t>(1));
    info.layerNum = std::max(textureInfo.layerNum, static_cast<uint16_t>(1));
    info.sampleNum = std::max(textureInfo.sampleNum, static_cast<uint8_t>(1));

    VkImageCreateInfo createInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    const FormatProperties& formatProps = getFormatProperties(textureInfo.format);

    VkImageCreateFlags flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT // typeless (basic)
        | VK_IMAGE_CREATE_EXTENDED_USAGE_BIT                      // typeless (advanced)
        | VK_IMAGE_CREATE_ALIAS_BIT;                              // matches https://learn.microsoft.com/en-us/windows/win32/direct3d12/memory-aliasing-and-data-inheritance#data-inheritance

    if (formatProps.blockWidth > 1 && (textureInfo.usage & TextureUsageBits::ShaderResourceStorage))
        flags |= VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT; // format can be used to create a view with an uncompressed format (1 texel covers 1 block)
    if (textureInfo.layerNum >= 6 && textureInfo.width == textureInfo.height)
        flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT; // allow cube maps
    if (textureInfo.dimension == TextureDimension::Dimension3D)
        flags |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT; // allow 3D demotion to a set of layers // TODO: hook up "VK_EXT_image_2d_view_of_3d"?
    // if (m_Desc.tiers.sampleLocations && formatProps.isDepth)
    //     flags |= VK_IMAGE_CREATE_SAMPLE_LOCATIONS_COMPATIBLE_DEPTH_BIT_EXT;

    createInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO}; // should be already set
    createInfo.flags = flags;
    // createInfo.imageType = ::GetImageType(textureDesc.type);
    // createInfo.format = ::GetVkFormat(textureDesc.format, true);
    // createInfo.extent.width = textureDesc.width;
    // createInfo.extent.height = std::max(textureDesc.height, (Dim_t)1);
    // createInfo.extent.depth = std::max(textureDesc.depth, (Dim_t)1);
    // createInfo.mipLevels = std::max(textureDesc.mipNum, (Dim_t)1);
    // createInfo.arrayLayers = std::max(textureDesc.layerNum, (Dim_t)1);
    // createInfo.samples = (VkSampleCountFlagBits)std::max(textureDesc.sampleNum, (Sample_t)1);
    // createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    // createInfo.usage = GetImageUsageFlags(textureDesc.usage);
    // createInfo.sharingMode = (m_NumActiveFamilyIndices <= 1 || textureDesc.sharingMode == SharingMode::EXCLUSIVE) ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT;
    // createInfo.queueFamilyIndexCount = m_NumActiveFamilyIndices;
    // createInfo.pQueueFamilyIndices = m_ActiveQueueFamilyIndices.data();
    // createInfo.initialLayout = IsMemoryZeroInitializationEnabled() ? VK_IMAGE_LAYOUT_ZERO_INITIALIZED_EXT : VK_IMAGE_LAYOUT_UNDEFINED;

    VULKAN_CHECK(vkCreateImage(device, &createInfo, &device.vulkanAllocationCallbacks, &image));
    return Result::Success;
  }

  void VulkanTexture::getMemoryInfo(MemoryLocation memoryLocation, MemoryInfo &memoryInfo) const {
    VkMemoryDedicatedRequirements dedicatedRequirements = {VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS};

    VkMemoryRequirements2 requirements = {VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2};
    requirements.pNext = &dedicatedRequirements;

    VkImageMemoryRequirementsInfo2 imageMemoryRequirements = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2};
    imageMemoryRequirements.image = image;

    vkGetImageMemoryRequirements2(device, &imageMemoryRequirements, &requirements);
    memoryInfo = {};
    device.getMemoryInfo(memoryLocation, requirements.memoryRequirements, dedicatedRequirements, memoryInfo);
  }
}
