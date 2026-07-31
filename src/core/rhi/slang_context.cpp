#include "slang_context.hpp"

namespace Core::Graphics {
  SlangContext::SlangContext() {
    SlangGlobalSessionDesc desc {};
    createGlobalSession(&desc, globalSession.writeRef());
    
    // TODO: Add runtime compilation
    slang::SessionDesc sessionDesc {};
    globalSession->createSession(sessionDesc, session.writeRef());
  }
}

