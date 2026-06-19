#pragma once
#include "../memory.hpp"

namespace Core {
  class Resource: public RefCounted {
    public:
      Resource() = default;

      ~Resource() override = default;
  };
}