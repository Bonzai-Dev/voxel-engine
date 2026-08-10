#pragma once

#if defined(BOOST_USE_UCONTEXT)
#include "fiber_ucontext.hpp"
#elif defined(BOOST_USE_WINFIB)
#include "fiber_winfib.hpp"
#else
#include "fiber_fcontext.hpp"
#endif
