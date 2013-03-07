// (C) Copyright Stephan Dollberg 2013. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef MEMCACHED_ASYNC_CLIENT_HPP
#define MEMCACHED_ASYNC_CLIENT_HPP

#include "config.hpp"
#include "detail/async_tcp_server.hpp"
#include "detail/consistent_hasher.hpp"

#include <boost/asio.hpp>

namespace memcachedcpp {
    template<typename T, ip ip_type, protocol protocol_type, typename hasher>
    class async_client_impl {};

    template<typename T, typename hasher>
    class async_client_impl<T, ip::tcp, protocol::plain, hasher> {
    public:
        async_client_impl(std::vector<std::string> new_servers, std::string new_port) : servers(std::make_move_iterator(new_servers.begin()), std::make_move_iterator(new_servers.end())), port(std::move(new_port)), con_hash(servers.begin(), servers.end()), server(servers.begin(), servers.end(), port) {
        }

        std::future<T> get(const std::string& key) {
        }

    private:
        std::set<std::string> servers;
        const std::string port;
        detail::consistent_hasher<hasher> con_hash;
        detail::async_tcp_server server;
    };

    template<typename T, ip ip_type, protocol protocol_type>
    using async_client = async_client_impl<T, ip_type, protocol_type, std::hash<std::string>>;


}

#endif // MEMCACHED_ASYNC_CLIENT_HPP
