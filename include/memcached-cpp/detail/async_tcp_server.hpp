// (C) Copyright Stephan Dollberg 2013. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef MEMCACHED_ASYNC_TCP_SERVER_HPP
#define MEMCACHED_ASYNC_TCP_SERVER_HPP

#include "asio_utils.hpp"
#include "memcached_utils.hpp"

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
            connect_n_tcp(sockets, service, begin, end, port);
            async_read_n();
            task = std::async(std::launch::async, [&] () { service.run(); });
        }

        void write(std::vector<char> buf, std::size_t index) {
            service.post(std::bind(&async_tcp_server::do_write, this, std::move(buf), index));
        }
        
        ~async_tcp_server() {
            service.stop();
        }

    private:
        boost::asio::io_service service;
        boost::ptr_vector<boost::asio::ip::tcp::socket> sockets;
        std::deque<std::tuple<std::size_t, std::vector<char>>> buffer;
        boost::asio::streambuf read_buffer;
        std::future<void> task;

        void do_write(std::vector<char> new_buffer, std::size_t index) {
            bool work_queue_empty = buffer.empty();
            buffer.emplace_back(std::make_tuple(index, std::move(new_buffer)));
            if(work_queue_empty) {
                async_write_wrapper();
            }
        }

        void async_read_n() {
            for(auto&& socket : sockets) {
                using namespace std::placeholders;
                boost::asio::async_read_until(socket, read_buffer, endmarker(), std::bind(&async_tcp_server::handle_read, this, _1, _2));
            }
        }

        void handle_read(const boost::system::error_code&, std::size_t) {
        }

        void handle_write(const boost::system::error_code&, std::size_t) {
            // error check
            buffer.pop_front();
            if(!buffer.empty()) {
                async_write_wrapper();
            }
        }

        void async_write_wrapper() {
            using namespace std::placeholders;
            boost::asio::async_write(sockets[std::get<0>(buffer.front())], boost::asio::buffer(std::get<1>(buffer.front())), std::bind(&async_tcp_server::handle_write, this, _1, _2));
        }
    };

}}

#endif // MEMCACHED_ASYNC_SERVER_HPP
