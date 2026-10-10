// © 2021 NVIDIA Corporation

MicromapVal::~MicromapVal() {
    if (m_Memory)
        m_Memory->Unbind(*this);

    Destroy(m_Buffer);
}

ENGINE_FORCE_INLINE uint64_t MicromapVal::GetBuildScratchBufferSize() const {
    return GetRayTracingInterfaceImpl().GetMicromapBuildScratchBufferSize(*GetImpl());
}

ENGINE_FORCE_INLINE uint64_t MicromapVal::GetNativeObject() const {
    NRI_RETURN_ON_FAILURE(&m_Device, IsBoundToMemory(), 0, "Micromap is not bound to memory");

    return GetRayTracingInterfaceImpl().GetMicromapNativeObject(GetImpl());
}

ENGINE_FORCE_INLINE Buffer* MicromapVal::GetBuffer() {
    NRI_RETURN_ON_FAILURE(&m_Device, IsBoundToMemory(), 0, "Micromap is not bound to memory");

    if (!m_Buffer) {
        Buffer* buffer = GetRayTracingInterfaceImpl().GetMicromapBuffer(*GetImpl());
        m_Buffer = Allocate<BufferVal>(m_Device.GetAllocationCallbacks(), m_Device, buffer, false);
    }

    return (Buffer*)m_Buffer;
}
