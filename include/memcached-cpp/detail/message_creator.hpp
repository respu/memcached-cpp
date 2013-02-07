// (C) Copyright Stephan Dollberg 2013. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef MEMCACHEDCPP_MESSAGE_CREATOR_HPP
#define MEMCACHEDCPP_MESSAGE_CREATOR_HPP

#include "utils.hpp"

#include <string>
#include <vector>
#include <iterator>
#include <type_traits>

namespace memcachedcpp { namespace detail {

    inline void fill_buffer(std::vector<char>& buffer, const std::string& t) {
        std::copy(std::begin(t), std::end(t), std::back_inserter(buffer));
    }

    template<typename ...Ts>
    void fill_buffer(std::vector<char>& buffer, const std::string& t, Ts&& ...ts) {
        std::copy(std::begin(t), std::end(t), std::back_inserter(buffer));
        buffer.push_back(' ');
        fill_buffer(buffer, std::forward<Ts>(ts)...);
    }
}}

#endif // MEMCACHEDCPP_MESSAGE_CREATOR_HPP
