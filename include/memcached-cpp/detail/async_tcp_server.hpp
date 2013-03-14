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
#include <functional>

#include <iostream>
namespace memcachedcpp { namespace detail {

    template<typename T>
    class async_tcp_server {
    public:
        template<typename server_iter>
        async_tcp_server(server_iter begin, server_iter end, const std::string& port) {
            connect_n_tcp(sockets, service, begin, end, port);
            async_read_n();
            task = std::async(std::launch::async, [&] () { service.run(); });
        }

        void write(std::vector<char> buf, std::size_t server_index) {
            service.post(std::bind(&async_tcp_server::do_write, this, std::move(buf), server_index));
        }

        std::future<T> get(std::vector<char> buf, std::size_t server_index) {
            std::promise<T> promise;
            auto fut = promise.get_future();
            std::lock_guard<std::mutex> lock(mutex);
            promises.emplace_back(std::make_tuple(server_index, std::move(promise)));
            service.post(std::bind(&async_tcp_server::do_get, this, std::move(buf), server_index));
            return fut;
        }
        
        ~async_tcp_server() {
            service.stop();
        }

    private:
        boost::asio::io_service service;
        boost::ptr_vector<boost::asio::ip::tcp::socket> sockets;
        std::deque<std::tuple<std::size_t, std::vector<char>>> buffer;
        std::deque<std::tuple<std::size_t, std::promise<T>>> promises;
        boost::asio::streambuf read_buffer;
        mutable std::mutex mutex;
        std::future<void> task;

        void do_write(std::vector<char> new_buffer, std::size_t index) {
            bool work_queue_empty = buffer.empty();
            buffer.emplace_back(std::make_tuple(index, std::move(new_buffer)));
            if(work_queue_empty) {
                async_write_wrapper();
            }
        }

        void do_get(std::vector<char> new_buffer, std::size_t index) {
            do_write(std::move(new_buffer), index);
        }


        void handle_read_header(const boost::system::error_code&, std::size_t bytes_read) {
            // TODO error check
            if(bytes_read == detail::endmarker_length()) { // no results found
                std::istream is(&read_buffer);
                is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                // TODO set error to promise
            }

            auto data_size = detail::extract_datasize(read_buffer);
            auto bytes_to_be_read = get_bytes_left(data_size, read_buffer);
            using namespace std::placeholders;
            boost::asio::async_read(
                sockets[std::get<0>(promises.front())], 
                read_buffer, 
                boost::asio::transfer_at_least(bytes_to_be_read),
                std::bind(&async_tcp_server::handle_read_data, this, data_size, _1, _2));
        }

        void handle_read_data(std::size_t data_size, const boost::system::error_code&, std::size_t) {
            using namespace std::placeholders;
            std::string data;
            decode_get(read_buffer, data_size, data);    

            boost::asio::async_read_until(sockets[std::get<0>(promises.front())], read_buffer, endmarker(),
                    std::bind(&async_tcp_server::handle_read_endmarker, this, _1, _2));

            std::get<1>(promises.front()).set_value(std::move(data));
        }

        void handle_read_endmarker(const boost::system::error_code&, std::size_t) {
            using namespace std::placeholders;
            auto server_id = std::get<0>(promises.front());
            boost::asio::async_read_until(sockets[server_id], read_buffer,
                    linefeed(), std::bind(&async_tcp_server::handle_read_header, this, _1, _2));

            std::istream is(&read_buffer);
            is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            promises.pop_front();
        }

        void async_read_n() {
            for(auto&& socket : sockets) {
                using namespace std::placeholders;
                boost::asio::async_read_until(socket, read_buffer, linefeed(), 
                        std::bind(&async_tcp_server::handle_read_header, this, _1, _2));
            }
        }

        void handle_write(const boost::system::error_code&, std::size_t) {
            // TODO error check
            buffer.pop_front();
            if(!buffer.empty()) {
                async_write_wrapper();
            }
        }

        void async_write_wrapper() {
            using namespace std::placeholders;
            boost::asio::async_write(sockets[std::get<0>(buffer.front())], boost::asio::buffer(std::get<1>(buffer.front())), 
                    std::bind(&async_tcp_server::handle_write, this, _1, _2));
        }

       
    };

}}

#endif // MEMCACHED_ASYNC_SERVER_HPP
