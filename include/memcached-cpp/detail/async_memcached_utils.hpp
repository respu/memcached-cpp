// (C) Copyright Stephan Dollberg 2013. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef MEMCACHED_ASYNC_MEMCACHED_UTILS_HPP_HPP
#define MEMCACHED_ASYNC_MEMCACHED_UTILS_HPP_HPP

#include "message_creator.hpp"
#include "memcached_utils.hpp"

namespace memcachedcpp { namespace detail {
    
    inline void async_encode_store(const std::string& command, const std::string& key, const std::string& value, std::size_t timeout, std::vector<char>& buffer) {
        auto timeout_str = boost::lexical_cast<std::string>(timeout);
        auto data_length = boost::lexical_cast<std::string>(value.length());

        fill_buffer(buffer, command, key, "0", timeout_str, data_length, noreply());
        fill_buffer(buffer, linefeed());
        fill_buffer(buffer, value);
        fill_buffer(buffer, linefeed());
    }
        

    inline void async_encode_get(const std::string& key, std::vector<char>& buffer) {
        encode_get(key, buffer);
    }

}}

#endif // MEMCACHED_ASYNC_MEMCACHED_UTILS_HPP_HPP
