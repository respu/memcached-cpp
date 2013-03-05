// (C) Copyright Stephan Dollberg 2013. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef MEMCACHEDCPP_MESSAGE_CREATOR_HPP
#define MEMCACHEDCPP_MESSAGE_CREATOR_HPP

#include "utils.hpp"
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext.hpp>

#include <string>
#include <vector>
#include <iterator>
#include <type_traits>

namespace memcachedcpp { namespace detail {

    inline void fill_buffer(std::vector<char>& buffer, const std::string& t) {
        boost::push_back(buffer, t);
    }

    template<typename ...Ts>
    void fill_buffer(std::vector<char>& buffer, const std::string& t, Ts&& ...ts) {
        boost::push_back(buffer, t);
        buffer.push_back(' ');
        fill_buffer(buffer, std::forward<Ts>(ts)...);
    }
}}

#endif // MEMCACHEDCPP_MESSAGE_CREATOR_HPP
