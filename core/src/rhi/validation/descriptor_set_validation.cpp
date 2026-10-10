// © 2021 NVIDIA Corporation

ENGINE_FORCE_INLINE void DescriptorSetVal::SetImpl(DescriptorSet* impl, const DescriptorSetDesc* desc) {
    m_Impl = impl;
    m_Desc = desc;
}

ENGINE_FORCE_INLINE void DescriptorSetVal::GetOffsets(uint32_t& resourceHeapOffset, uint32_t& samplerHeapOffset) const {
    GetCoreInterfaceImpl().GetDescriptorSetOffsets(*GetImpl(), resourceHeapOffset, samplerHeapOffset);
}

