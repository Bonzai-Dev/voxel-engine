// © 2021 NVIDIA Corporation

ENGINE_FORCE_INLINE void DescriptorSetVK::SetDebugName(const char* name) {
    m_Device->SetDebugNameToTrivialObject(VK_OBJECT_TYPE_DESCRIPTOR_SET, (uint64_t)m_Handle, name);
}
