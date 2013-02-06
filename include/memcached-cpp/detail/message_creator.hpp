// (C) Copyright Stephan Dollberg 2013. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef MEMCACHEDCPP_MESSAGE_CREATOR_HPP
#define MEMCACHEDCPP_MESSAGE_CREATOR_HPP

#include "utils.hpp"

#include <vector>
#include <iterator>
#include <type_traits>

namespace memcachedcpp { namespace detail {

    template<typename T, DisableIf<std::is_array<typename std::remove_reference<T>::type>> = _>
    void fill_buffer(std::vector<char>& buffer, T&& t) {
        std::copy(std::begin(t), std::end(t), std::back_inserter(buffer));
    }

    template<typename T, EnableIf<std::is_array<typename std::remove_reference<T>::type>> = _>
    void fill_buffer (std::vector<char>& buffer, T&& t) {
        std::copy(std::begin(t), std::end(t) - 1, std::back_inserter(buffer));
    }

    template<typename T, typename ...Ts, DisableIf<std::is_array<typename std::remove_reference<T>::type>> = _>
    void fill_buffer(std::vector<char>& buffer, T&& t, Ts&& ...ts) {
        std::copy(std::begin(t), std::end(t), std::back_inserter(buffer));
        buffer.push_back(' ');
        fill_buffer(buffer, ts...);
    }

    template<typename T, typename ...Ts, EnableIf<std::is_array<typename std::remove_reference<T>::type>> = _>
    void fill_buffer (std::vector<char>& buffer, T&& t, Ts&& ...ts) {
        std::copy(std::begin(t), std::end(t) - 1, std::back_inserter(buffer));
        buffer.push_back(' ');
        fill_buffer(buffer, ts...);
    }
}}

#endif // MEMCACHEDCPP_MESSAGE_CREATOR_HPP
