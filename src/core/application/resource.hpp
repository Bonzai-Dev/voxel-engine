#pragma once
#include "../pointers.hpp"

namespace Core {
  class Resource: public RefCounted {
    public:
      Resource() = default;

      ~Resource() override = default;
  };
}