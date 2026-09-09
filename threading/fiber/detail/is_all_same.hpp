//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)


//          Copyright Oliver Kowalke 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_FIBERS_DETAIL_IS_ALL_SAME_H
#define BOOST_FIBERS_DETAIL_IS_ALL_SAME_H

#include <type_traits>

#include <boost/config.hpp>

#include <boost/fiber/detail/config.hpp>



namespace boost {
namespace fibers {
namespace detail {

template< typename X, typename ... Y >
struct is_all_same;

template< typename X, typename Y0, typename ... Y >
struct is_all_same< X, Y0, Y ... > {
    static constexpr bool value =
        std::is_same< X, Y0 >::value && is_all_same< X, Y ... >::value;
};

template< typename X, typename Y0 >
struct is_all_same< X, Y0 > {
    static constexpr bool value = std::is_same< X, Y0 >::value;
};

}}}



#endif // BOOST_FIBERS_DETAIL_IS_ALL_SAME_H
