#include "stl/allocator.hpp"
#include "rhi.hpp"

namespace Core::RHI {
  Device::Device(const CallbackInterface &callbacks, const AllocationCallbacks &allocationCallbacks):
  callbackInterface(callbacks), allocationCallbacks(allocationCallbacks), stdAllocator(allocationCallbacks) {
  }
}