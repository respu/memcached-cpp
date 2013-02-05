// (C) Copyright Stephan Dollberg 2013. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef MEMCACHEDCPP_MESSAGE_CREATOR_HPP
#define MEMCACHEDCPP_MESSAGE_CREATOR_HPP

#include <vector>
#include <iterator>
#include <type_traits>

namespace memcachedcpp { namespace detail {
    template<typename T, typename std::enable_if<!std::is_array<typename std::remove_reference<T>::type>::value, int>::type = 0>
    inline void indices(std::vector<char>& buffer, T&& t) {
        std::copy(std::begin(t), std::end(t), std::back_inserter(buffer));
    }

    template<typename T, typename std::enable_if<std::is_array<typename std::remove_reference<T>::type>::value, int>::type = 0>
    inline void indices (std::vector<char>& buffer, T&& t) {
        std::copy(std::begin(t), std::end(t) - 1, std::back_inserter(buffer));
    }

    template<typename T, typename ...Ts, typename std::enable_if<!std::is_array<typename std::remove_reference<T>::type>::value, int>::type = 0>
    inline void indices(std::vector<char>& buffer, T&& t, Ts&& ...ts) {
        std::copy(std::begin(t), std::end(t), std::back_inserter(buffer));
        buffer.push_back(' ');
        indices(buffer, ts...);
    }

    template<typename T, typename ...Ts, typename std::enable_if<std::is_array<typename std::remove_reference<T>::type>::value, int>::type = 0>
    inline void indices (std::vector<char>& buffer, T&& t, Ts&& ...ts) {
        std::copy(std::begin(t), std::end(t) - 1, std::back_inserter(buffer));
        buffer.push_back(' ');
        indices(buffer, ts...);
    }
}}

#endif // MEMCACHEDCPP_MESSAGE_CREATOR_HPP
