// © 2021 NVIDIA Corporation
#include <core/core.hpp>
#include <core/rhi/rhi.hpp>
#include "buffer_validation.hpp"

namespace Core::RHI {
  BufferVal::~BufferVal() {
    if (m_Memory)
      m_Memory->Unbind(*this);
  }

  ENGINE_FORCE_INLINE void *BufferVal::Map(uint64_t offset, uint64_t size) {
    if (size == WHOLE_SIZE)
      size = m_Desc.size;

    NRI_RETURN_ON_FAILURE(&m_Device, m_IsBoundToMemory, nullptr, "the buffer is not bound to memory");
    NRI_RETURN_ON_FAILURE(&m_Device, !m_IsMapped, nullptr,
                          "the buffer is already mapped (D3D11 doesn't support nested calls)");
    NRI_RETURN_ON_FAILURE(&m_Device, offset + size <= m_Desc.size, nullptr, "out of bounds");

    m_IsMapped = true;

    return GetCoreInterfaceImpl().mapBuffer(*GetImpl(), offset, size);
  }

  ENGINE_FORCE_INLINE void BufferVal::Unmap() {
    NRI_RETURN_ON_FAILURE(&m_Device, m_IsMapped, ReturnVoid(), "the buffer is not mapped");

    m_IsMapped = false;

    GetCoreInterfaceImpl().unmapBuffer(*GetImpl());
  }

  ENGINE_FORCE_INLINE uint64_t BufferVal::GetDeviceAddress() const {
    return GetCoreInterfaceImpl().getBufferDeviceAddress(*GetImpl());
  }
}
