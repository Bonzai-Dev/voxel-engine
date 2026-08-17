#pragma once

#include <cstdint>
#include <set>
#include <vector>
// #include <boost/config.hpp>
#include <core/threading/fiber/detail/config.hpp>

// #ifdef BOOST_HAS_ABI_HEADERS
// # include BOOST_ABI_PREFIX
// #endif

namespace Core {
  namespace Fibers {
    namespace numa {
      struct node {
        std::uint32_t id;
        std::set<std::uint32_t> logical_cpus;
        std::vector<std::uint32_t> distance;
      };

      inline bool operator<(node const &lhs, node const &rhs) noexcept {
        return lhs.id < rhs.id;
      }

      std::vector<node> topology();
    }
  }
}

// #ifdef BOOST_HAS_ABI_HEADERS
// # include BOOST_ABI_SUFFIX
// #endif
