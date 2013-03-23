// (C) Copyright Stephan Dollberg 2013. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef MEMCACHED_ASYNC_CLIENT_HPP
#define MEMCACHED_ASYNC_CLIENT_HPP

#include "config.hpp"
#include "detail/async_tcp_server.hpp"
#include "detail/consistent_hasher.hpp"
#include "detail/message_creator.hpp"
#include "detail/async_memcached_utils.hpp"

#include <boost/asio.hpp>

namespace memcachedcpp {
    template<typename Datatype, ip ip_type, protocol protocol_type, typename hasher>
    class async_client_impl {};

    template<typename Datatype, typename hasher>
    class async_client_impl<Datatype , ip::tcp, protocol::plain, hasher> {
    public:
        async_client_impl(std::vector<std::string> new_servers, std::string new_port) : servers(std::make_move_iterator(new_servers.begin()), std::make_move_iterator(new_servers.end())), port(std::move(new_port)), con_hash(servers.begin(), servers.end()), server(servers.begin(), servers.end(), port) {
        }

        void set(const std::string& key, const Datatype& value, std::size_t timeout = 0) {
            detail::async_encode_store("set", key, value, timeout, write_buffer);
            auto server_id = con_hash.get_node_id(key);
            server.write(write_buffer, server_id);
        }

        std::future<Datatype> get(const std::string& key) {
            detail::async_encode_get(key, write_buffer);
            auto server_id = con_hash.get_node_id(key);
            auto fut = server.get(write_buffer, server_id);
            return fut;
        }

    private:
        std::set<std::string> servers;
        const std::string port;
        detail::consistent_hasher<hasher> con_hash;
        std::vector<char> write_buffer;
        detail::async_tcp_server<Datatype> server;
    };

    template<typename T, ip ip_type, protocol protocol_type, typename hasher = std::hash<std::string>>
    using async_client = async_client_impl<T, ip_type, protocol_type, hasher>;


}

#endif // MEMCACHED_ASYNC_CLIENT_HPP
