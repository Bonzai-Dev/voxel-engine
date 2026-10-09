// © 2021 NVIDIA Corporation

#pragma once
#include "SharedExternal.hpp"

namespace Core::RHI {
  /*
  TODO: inheritance is a bit tricky:
  - "Objects => DebugNameBase"
  - "Device => DeviceBase => DebugNameBaseVal"
  - "ObjectVal => DebugNameBaseVal"
  Why?
  - validation objects should always have names (they ignore "NRI_ENABLE_DEBUG_NAMES_AND_ANNOTATIONS" state)
  - non-validation objects should get "-8" bytes to their sizes if "NRI_ENABLE_DEBUG_NAMES_AND_ANNOTATIONS = 0" (-1 virtual function)
  Notes:
  - "DebugNameBaseVal" is used only inside validation
  - "DebugNameBase" is used only inside implementations (gets transformed to NOP if "NRI_ENABLE_DEBUG_NAMES_AND_ANNOTATIONS = 0")
  - implementations don't call "SetDebugName" from base classes if "NRI_ENABLE_DEBUG_NAMES_AND_ANNOTATIONS = 0"
  - "DebugNameBaseVal" can be cast to "DebugNameBase" if "NRI_ENABLE_DEBUG_NAMES_AND_ANNOTATIONS = 1" (it happens for device in implementations)
  */
#if NRI_ENABLE_DEBUG_NAMES_AND_ANNOTATIONS
#    define NRI_DEBUG_NAME_OVERRIDE override

  struct DebugNameBase {
    virtual void SetDebugName(const char *) {
    }
  };

#else
#    define NRI_DEBUG_NAME_OVERRIDE

  struct DebugNameBase {
  };

#endif

  struct DebugNameBaseVal {
    virtual void SetDebugName(const char *) {
    }
  };

  struct DeviceBase: public DebugNameBaseVal {
    inline DeviceBase(const CallbackInterface &callbacks, const AllocationCallbacks &allocationCallbacks,
                      uint64_t signature = 0)
      : m_CallbackInterface(callbacks)
        , m_AllocationCallbacks(allocationCallbacks)
        , m_StdAllocator(m_AllocationCallbacks) {
#ifndef NDEBUG
      m_Signature = signature;
#else
      MaybeUnused(signature);
#endif
    }

    inline StdAllocator<uint8_t> &GetStdAllocator() {
      return m_StdAllocator;
    }

    inline const AllocationCallbacks &GetAllocationCallbacks() const {
      return m_AllocationCallbacks;
    }

    void ReportMessage(Message messageType, Result result, const char *file, uint32_t line, const char *format,
                       ...) const;

    // Pure virtual
    virtual const DeviceInfo &GetDesc() const = 0;
    virtual void Destruct() = 0;

    // Virtual
    virtual ~DeviceBase() {
    }

    protected:
#ifndef NDEBUG
      uint64_t m_Signature = 0; // .natvis
#endif
      CallbackInterface m_CallbackInterface = {};
      AllocationCallbacks m_AllocationCallbacks = {};
      StdAllocator<uint8_t> m_StdAllocator;

    private:
      virtual Result loadInterface(Device &device, CoreInterface &coreInterface) { return Result::Unsupported; }
      virtual Result loadInterface(Device &device, SwapChainInterface &coreInterface) { return Result::Unsupported; }

      friend Result getInterface(Device &device, CoreInterface &coreInterface);
      friend Result getInterface(Device &device, SwapChainInterface &coreInterface);
  };

  template <typename T>
  inline void Destroy(T *object) {
    if (object) {
      object->~T();

      const auto &allocationCallbacks = ((DeviceBase&)(object->GetDevice())).GetAllocationCallbacks();
      allocationCallbacks.Free(allocationCallbacks.userArg, object);
    }
  }
}
