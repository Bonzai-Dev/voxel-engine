#pragma once
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>
// #include <boost/config.hpp>
#include <core/threading/context/detail/config.hpp>
#if defined(BOOST_NO_CXX17_STD_INVOKE)
#include <boost/context/detail/invoke.hpp>
#endif
#include <core/threading/context/detail/index_sequence.hpp>

// #ifdef BOOST_HAS_ABI_HEADERS
// # include BOOST_ABI_PREFIX
// #endif

#if defined(ENGINE_COMPILER_MSVC)
# pragma warning(push)
# pragma warning(disable: 4100)
#endif

namespace Core::Context::detail {
  template <typename Fn, typename Tpl, std::size_t ... I>
  auto apply_impl(Fn &&fn, Tpl &&tpl, index_sequence<I...>)
#if defined(BOOST_NO_CXX17_STD_INVOKE)
  -> decltype( Core::Context::detail::invoke(std::forward<Fn>(fn), std::get<I>(std::forward<Tpl>(tpl))...) )
#else
    -> decltype( std::invoke(std::forward<Fn>(fn), std::get<I>(std::forward<Tpl>(tpl))...) )
#endif
  {
#if defined(BOOST_NO_CXX17_STD_INVOKE)
    return Core::Context::detail::invoke(std::forward<Fn>(fn), std::get<I>(std::forward<Tpl>(tpl))...);
#else
    return std::invoke(std::forward<Fn>(fn), std::get<I>(std::forward<Tpl>(tpl))...);
#endif
  }

  template <typename Fn, typename Tpl>
  auto apply(Fn &&fn, Tpl &&tpl)
    -> decltype( apply_impl(std::forward<Fn>(fn),
                            std::forward<Tpl>(tpl),
                            make_index_sequence<std::tuple_size<typename std::decay<Tpl>::type>::value>{}) ) {
    return apply_impl(std::forward<Fn>(fn),
                      std::forward<Tpl>(tpl),
                      make_index_sequence<std::tuple_size<typename std::decay<Tpl>::type>::value>{});
  }
}

#if defined(ENGINE_COMPILER_MSVC)
# pragma warning(pop)
#endif

// #ifdef BOOST_HAS_ABI_HEADERS
// #include BOOST_ABI_SUFFIX
// #endif
