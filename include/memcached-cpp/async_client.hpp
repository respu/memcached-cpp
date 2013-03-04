// (C) Copyright Stephan Dollberg 2013. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef MEMCACHED_ASYNC_CLIENT_HPP
#define MEMCACHED_ASYNC_CLIENT_HPP

#include "config.hpp"

namespace memcachedcpp {
    template<typename T, ip ip_type, protocol protocol_type, typename hasher>
    class async_client {};

    template<typename T, typename hasher>
    class async_client<T, ip::tcp, protocol::plain, hasher> {
    public:

    private:
    };
}

#endif // MEMCACHED_ASYNC_CLIENT_HPP
