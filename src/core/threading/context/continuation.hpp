#pragma once

#if defined(BOOST_USE_UCONTEXT)
#include "continuation_ucontext.hpp"
#elif defined(BOOST_USE_WINFIB)
#include "continuation_winfib.hpp"
#else
#include "continuation_fcontext.hpp"
#endif
