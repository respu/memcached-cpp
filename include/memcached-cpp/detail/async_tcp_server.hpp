// (C) Copyright Stephan Dollberg 2013. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef MEMCACHED_ASYNC_TCP_SERVER_HPP
#define MEMCACHED_ASYNC_TCP_SERVER_HPP

#include "asio_utils.hpp"

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/asio.hpp>

#include <string>
#include <future>

namespace memcachedcpp { namespace detail {

    class async_tcp_server {
    public:
        template<typename server_iter>
        async_tcp_server(server_iter begin, server_iter end, const std::string& port) {
            connect_n_tcp(sockets, service, begin, end, port);
        }

        void write(std::vector<char>& buf) {
        }
        
    private:
        boost::asio::io_service service;
        boost::ptr_vector<boost::asio::ip::tcp::socket> sockets;
        std::future<void> task;
    };

}}

#endif // MEMCACHED_ASYNC_SERVER_HPP
