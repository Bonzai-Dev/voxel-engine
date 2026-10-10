#include <cstdint>
#include <core/core.hpp>
#include <vulkan/vulkan.h>
#include "descriptor_set_vulkan.hpp"

namespace Core::RHI {
  ENGINE_FORCE_INLINE void DescriptorSetVK::SetDebugName(const char *name) {
    m_Device->SetDebugNameToTrivialObject(VK_OBJECT_TYPE_DESCRIPTOR_SET, (uint64_t)m_Handle, name);
  }
}
