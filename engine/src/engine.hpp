#pragma once
#include <engine/core/assert.hpp>
#ifdef ENGINE_PLATFORM_WINDOWS
  void maint();
#endif

namespace Engine::Core {
  ENGINE_DEFINE_LOG_CATEGORY(coreLogger, "CORE");

}