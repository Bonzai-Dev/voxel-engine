// © 2021 NVIDIA Corporation

namespace Core::RHI {
  AccelerationStructureVal::~AccelerationStructureVal() {
    if (m_Memory)
      m_Memory->Unbind(*this);

    Destroy(m_Buffer);
  }

  ENGINE_FORCE_INLINE uint64_t AccelerationStructureVal::GetUpdateScratchBufferSize() const {
    return GetRayTracingInterfaceImpl().GetAccelerationStructureUpdateScratchBufferSize(*GetImpl());
  }

  ENGINE_FORCE_INLINE uint64_t AccelerationStructureVal::GetBuildScratchBufferSize() const {
    return GetRayTracingInterfaceImpl().GetAccelerationStructureBuildScratchBufferSize(*GetImpl());
  }

  ENGINE_FORCE_INLINE uint64_t AccelerationStructureVal::GetHandle() const {
    NRI_RETURN_ON_FAILURE(&m_Device, IsBoundToMemory(), 0, "AccelerationStructure is not bound to memory");

    return GetRayTracingInterfaceImpl().GetAccelerationStructureHandle(*GetImpl());
  }

  ENGINE_FORCE_INLINE uint64_t AccelerationStructureVal::GetNativeObject() const {
    NRI_RETURN_ON_FAILURE(&m_Device, IsBoundToMemory(), 0, "AccelerationStructure is not bound to memory");

    return GetRayTracingInterfaceImpl().GetAccelerationStructureNativeObject(GetImpl());
  }

  ENGINE_FORCE_INLINE Buffer *AccelerationStructureVal::GetBuffer() {
    NRI_RETURN_ON_FAILURE(&m_Device, IsBoundToMemory(), 0, "AccelerationStructure is not bound to memory");

    if (!m_Buffer) {
      Buffer *buffer = GetRayTracingInterfaceImpl().GetAccelerationStructureBuffer(*GetImpl());
      m_Buffer = Allocate<BufferVal>(m_Device.GetAllocationCallbacks(), m_Device, buffer, false);
    }

    return (Buffer*)m_Buffer;
  }

  ENGINE_FORCE_INLINE Result AccelerationStructureVal::CreateDescriptor(Descriptor *&descriptor) {
    NRI_RETURN_ON_FAILURE(&m_Device, IsBoundToMemory(), Result::InvalidArgument,
                          "AccelerationStructure is not bound to memory");

    Descriptor *descriptorImpl = nullptr;
    const Result result = GetRayTracingInterfaceImpl().CreateAccelerationStructureDescriptor(*GetImpl(), descriptorImpl);

    descriptor = nullptr;
    if (result == Result::Success)
      descriptor = (Descriptor*)Allocate<DescriptorVal>(m_Device.GetAllocationCallbacks(), m_Device, descriptorImpl,
                                                        DescriptorType::AccelerationStructure);

    return result;
  }
}