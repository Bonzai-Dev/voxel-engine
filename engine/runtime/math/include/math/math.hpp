#pragma once
#include <cstdint>

namespace Core::Math {
}

namespace Core::Math::Units {
  constexpr uint64_t msToUs(uint32_t x) {
    return x * 1000000ull;
  }
}
