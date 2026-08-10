#pragma once
#include <tuple>
#include <utility>
#include "index_sequence.hpp"
#include "config.hpp"

namespace Core::Context::detail {
  template<typename... S, typename... T, std::size_t ... I>
  void head_impl(std::tuple<S...> &s,
                 std::tuple<T...> &t, index_sequence<I...>) {
    t = std::tuple<T...>{std::get<I>(s)...};
  }

  template<typename... S, typename... T, std::size_t ... I>
  void head_impl(std::tuple<S...> &&s,
                 std::tuple<T...> &t, index_sequence<I...>) {
    t = std::tuple<T...>{std::get<I>(std::move(s))...};
  }

  template<typename... S, std::size_t ... I1, typename... T, std::size_t ... I2>
  void tail_impl(std::tuple<S...> &s, index_sequence<I1...>,
                 std::tuple<T...> &t, index_sequence<I2...>) {
    constexpr std::size_t Idx = (sizeof...(I1)) - (sizeof...(I2));
    t = std::tuple<T...>{std::get<(Idx + I2)>(s)...};
  }

  template<typename... S, std::size_t ... I1, typename... T, std::size_t ... I2>
  void tail_impl(std::tuple<S...> &&s, index_sequence<I1...>, std::tuple<T...> &t, index_sequence<I2...>) {
    constexpr std::size_t Idx = (sizeof...(I1)) - (sizeof...(I2));
    t = std::tuple<T...>{std::get<(Idx + I2)>(std::move(s))...};
  }

  template<typename... T>
  class tuple_head;

  template<typename... T>
  class tuple_head<std::tuple<T...> > {
    public:
      tuple_head(std::tuple<T...> &t) noexcept: t_(t) {
      }

      template<typename... S>
      void operator=(std::tuple<S...> &s) {
        static_assert((sizeof...(T)) <= (sizeof...(S)), "invalid tuple size");
        head_impl(s,
                  t_, index_sequence_for<T...>{});
      }

      template<typename... S>
      void operator=(std::tuple<S...> &&s) {
        static_assert((sizeof...(T)) <= (sizeof...(S)), "invalid tuple size");
        head_impl(std::move(s),
                  t_, index_sequence_for<T...>{});
      }

    private:
      std::tuple<T...> &t_;
  };

  template<typename... T>
  class tuple_tail;

  template<typename... T>
  class tuple_tail<std::tuple<T...> > {
    public:
      tuple_tail(std::tuple<T...> &t) noexcept: t_(t) {
      }

      template<typename... S>
      void operator=(std::tuple<S...> &s) {
        static_assert((sizeof...(T)) <= (sizeof...(S)), "invalid tuple size");
        tail_impl(s, index_sequence_for<S...>{},
                  t_, index_sequence_for<T...>{});
      }

      template<typename... S>
      void operator=(std::tuple<S...> &&s) {
        static_assert((sizeof...(T)) <= (sizeof...(S)), "invalid tuple size");
        tail_impl(std::move(s), index_sequence_for<S...>{},
                  t_, index_sequence_for<T...>{});
      }

    private:
      std::tuple<T...> &t_;
  };

  template<typename... T>
  detail::tuple_head<std::tuple<T...> > head(std::tuple<T...> &tpl) {
    return tuple_head<std::tuple<T...> >{tpl};
  }

  template<typename... T>
  detail::tuple_tail<std::tuple<T...> > tail(std::tuple<T...> &tpl) {
    return tuple_tail<std::tuple<T...> >{tpl};
  }
}
