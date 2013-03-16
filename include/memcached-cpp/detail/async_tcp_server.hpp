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
        async_tcp_server(server_iter begin, server_iter end, const std::string& port) : 
            promises(std::distance(begin,end)), 
            read_buffers(std::distance(begin,end))
        {
            connect_n_tcp(sockets, service, begin, end, port);
            async_read_n();
            task = std::async(std::launch::async, [&] () { service.run(); });
        }

        void write(std::vector<char> buf, std::size_t server_index) {
            service.post(std::bind(&async_tcp_server::do_write, this, std::move(buf), server_index));
        }

        std::future<T> get(std::vector<char> buf, std::size_t server_id) {
            std::promise<T> promise;
            auto fut = promise.get_future();
            std::lock_guard<std::mutex> lock(mutex);
            promises[server_id].emplace_back(std::move(promise));
            service.post(std::bind(&async_tcp_server::do_get, this, std::move(buf), server_id));
            return fut;
        }
        
        ~async_tcp_server() {
            service.stop();
        }

    private:
        boost::asio::io_service service;
        boost::ptr_vector<boost::asio::ip::tcp::socket> sockets;
        std::deque<std::tuple<std::size_t, std::vector<char>>> buffer;
        std::deque<std::deque<std::promise<T>>> promises;
        std::deque<boost::asio::streambuf> read_buffers;
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


        void handle_read_header(std::size_t server_id, const boost::system::error_code& , std::size_t bytes_read) {
            // TODO error check
            if(bytes_read == detail::endmarker_length()) { // no results found
                std::istream is(&read_buffers[server_id]);
                is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                // TODO set error to promise
                assert(true);
            }

            auto data_size = detail::extract_datasize(read_buffers[server_id]);
            auto bytes_to_be_read = get_bytes_left(data_size, read_buffers[server_id]);
            using namespace std::placeholders;
            boost::asio::async_read(
                sockets[server_id], 
                read_buffers[server_id], 
                boost::asio::transfer_at_least(bytes_to_be_read),
                std::bind(&async_tcp_server::handle_read_data, this, server_id, data_size, _1, _2));
        }

        void handle_read_data(std::size_t server_id, std::size_t data_size, const boost::system::error_code&, std::size_t) {
            using namespace std::placeholders;
            std::string data;
            decode_get(read_buffers[server_id], data_size, data);    

            boost::asio::async_read_until(sockets[server_id], read_buffers[server_id], endmarker(),
                    std::bind(&async_tcp_server::handle_read_endmarker, this, server_id, _1, _2));

            promises[server_id].front().set_value(std::move(data));
        }

        void handle_read_endmarker(std::size_t server_id, const boost::system::error_code&, std::size_t) {
            std::istream is(&read_buffers[server_id]);
            is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            using namespace std::placeholders;
            boost::asio::async_read_until(sockets[server_id], read_buffers[server_id],
                    linefeed(), std::bind(&async_tcp_server::handle_read_header, this, server_id, _1, _2));

            promises[server_id].pop_front();
        }

        void async_read_n() {
            for(auto&& server_id : boost::irange<std::size_t>(0, sockets.size())) {
                using namespace std::placeholders;
                boost::asio::async_read_until(sockets[server_id], read_buffers[server_id], linefeed(), 
                        std::bind(&async_tcp_server::handle_read_header, this, server_id, _1, _2));
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
