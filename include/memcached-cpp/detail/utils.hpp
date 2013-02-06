// (C) Copyright Stephan Dollberg 2013. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef MEMCACHEDCPP_UTILS_CPP
#define MEMCACHEDCPP_UTILS_CPP

#include <type_traits>

namespace memcachedcpp { 

    // TMP stuff, thanks to the master Martinho Fernandes
    // for more info check http://flamingdangerzone.com/cxx11/2012/06/01/almost-static-if.html
    // and https://bitbucket.org/martinhofernandes/wheels
    
    namespace meta_detail {
        enum class enabler { _ };
    } 

    constexpr auto _ = meta_detail::enabler::_;

    template <typename Condition>
    using EnableIf = typename std::enable_if<Condition::value, decltype(_)>::type;

    template <typename Condition>
    using DisableIf = typename std::enable_if<!Condition::value, decltype(_)>::type;

}

#endif // MEMCACHEDCPP_UTILS_CPP
