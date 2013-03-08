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
#include <vector>
#include <deque>

#include <iostream>
namespace memcachedcpp { namespace detail {

    class async_tcp_server {
    public:
        template<typename server_iter>
        async_tcp_server(server_iter begin, server_iter end, const std::string& port) {
            async_connect_n_tcp(sockets, service, begin, end, port, [] (const boost::system::error_code&, boost::asio::ip::tcp::resolver::iterator) { });
            //task = std::async(std::launch::async, [&] () { std::cout << service.run(); });
            service.run();
            std::cout << "afsdf" << std::endl; 
        }

        //void write(std::vector<char>& buf, std::size_t index) {
        //}
        
    private:
        boost::asio::io_service service;
        boost::ptr_vector<boost::asio::ip::tcp::socket> sockets;
        std::deque<std::vector<char>> buffer;
        std::future<void> task;

        void do_write(std::vector<char> new_buffer, std::size_t index) {
            bool work_queue_empty = buffer.empty();
            buffer.emplace_back(std::move(new_buffer));
            if(work_queue_empty) {
             
            }
        }

        void handle_connect(const boost::system::error_code& error, boost::asio::ip::tcp::resolver::iterator) {

        }
    };

}}

#endif // MEMCACHED_ASYNC_SERVER_HPP
