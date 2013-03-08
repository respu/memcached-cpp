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

    inline boost::asio::ip::tcp::resolver::iterator generate_tcp_endpoint(boost::asio::io_service& service, const std::string& server_name, const std::string& port) {
        boost::asio::ip::tcp::resolver resolver(service);
        boost::asio::ip::tcp::resolver::query query(server_name, port);
        return resolver.resolve(query);
    }

    template<typename server_iter>
    void connect_n_tcp(boost::ptr_vector<tcp_socket>& sockets, boost::asio::io_service& service, server_iter begin, server_iter end, const std::string& port) {
        for(auto iter = begin; iter != end; ++iter) {
            std::unique_ptr<tcp_socket> socket_ptr(new tcp_socket(service));
            boost::asio::connect(*socket_ptr, generate_tcp_endpoint(service, *iter, port));
            sockets.push_back(socket_ptr.get());
            socket_ptr.release();
        }
    }

    template<typename server_iter, typename Callback>
    void async_connect_n_tcp(boost::ptr_vector<tcp_socket>& sockets, boost::asio::io_service& service, server_iter begin, server_iter end, const std::string& port, Callback&& cb) {
        for(auto iter = begin; iter != end; ++iter) {
            std::unique_ptr<tcp_socket> socket_ptr(new tcp_socket(service));
            boost::asio::async_connect(*socket_ptr, generate_tcp_endpoint(service, *iter, port), std::forward<Callback>(cb));
            sockets.push_back(socket_ptr.get());
            socket_ptr.release();
        }
    }
}}

#endif // MEMCACHED_ASIO_UTILS_HPP
