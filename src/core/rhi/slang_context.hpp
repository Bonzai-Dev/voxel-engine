#pragma once
#include "slang-com-ptr.h"

namespace Core::Graphics {
  class SlangContext {
    public:
      SlangContext();

    private:
      Slang::ComPtr<slang::IGlobalSession> globalSession;
      Slang::ComPtr<slang::ISession> session;

  };
}
