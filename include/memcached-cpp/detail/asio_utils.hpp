// (C) Copyright Stephan Dollberg 2013. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef MEMCACHED_ASIO_UTILS_HPP
#define MEMCACHED_ASIO_UTILS_HPP

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/asio.hpp>

#include <memory>
#include <string>

namespace memcachedcpp { namespace detail {
    using tcp_socket = boost::asio::ip::tcp::socket;

    template<typename server_iter>
    void connect_n_tcp(boost::ptr_vector<tcp_socket>& sockets, boost::asio::io_service& service, server_iter begin, server_iter end, const std::string& port) {
        for(auto iter = begin; iter != end; ++iter) {
            std::unique_ptr<tcp_socket> socket_ptr(new tcp_socket(service));
            boost::asio::ip::tcp::resolver resolver(service);
            boost::asio::ip::tcp::resolver::query query(*iter, port);
            auto endpoint_iter = resolver.resolve(query);
            boost::asio::connect(*socket_ptr, endpoint_iter);
            sockets.push_back(socket_ptr.release());
        }
    }
}}

#endif // MEMCACHED_ASIO_UTILS_HPP
