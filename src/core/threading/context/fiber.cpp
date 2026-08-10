#include <core/assert.hpp>

#if defined(BOOST_USE_UCONTEXT)
#include "boost/context/fiber_ucontext.hpp"
#elif defined(BOOST_USE_WINFIB)
#include "boost/context/fiber_winfib.hpp"
#endif

#include <boost/config.hpp>

namespace Core::Context::detail {
  // zero-initialization
  thread_local fiber_activation_record *fib_current_rec;
  thread_local static std::size_t counter;

  // schwarz counter
  fiber_activation_record_initializer::fiber_activation_record_initializer() noexcept {
    if (0 == counter++) {
      fib_current_rec = new fiber_activation_record();
    }
  }

  fiber_activation_record_initializer::~fiber_activation_record_initializer() {
    if (0 == --counter) {
      ENGINE_STATIC_ASSERT(fib_current_rec->is_main_context());
      delete fib_current_rec;
    }
  }
}

namespace detail {
  fiber_activation_record *&fiber_activation_record::current() noexcept {
    // initialized the first time control passes; per thread
    thread_local static fiber_activation_record_initializer initializer;
    return fib_current_rec;
  }
}
