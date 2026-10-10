// © 2021 NVIDIA Corporation

ENGINE_FORCE_INLINE uint64_t FenceVal::GetFenceValue() const {
    return GetCoreInterfaceImpl().GetFenceValue(*GetImpl());
}

ENGINE_FORCE_INLINE void FenceVal::Wait(uint64_t value) {
    GetCoreInterfaceImpl().Wait(*GetImpl(), value);
}
