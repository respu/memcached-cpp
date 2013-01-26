// (C) Copyright Stephan Dollberg 2013. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "detail/memcached_utils.hpp"

#include "boost/asio.hpp"
#include <string>
#include <vector>
#include <utility>

namespace memcachedcpp {
    template<typename Datatype>
    class client {
    public:
        client(std::vector<std::string> servers, std::string port) 
            : servers(std::move(servers)), port(std::move(port)), socket(service) 
        {
            connect();
        }

        void connect() {
            boost::asio::ip::tcp::resolver resolver(service);
            boost::asio::ip::tcp::resolver::query query(servers[0], port);
            auto endpoint_iter = resolver.resolve(query);
            boost::asio::connect(socket, endpoint_iter);
        }

        std::string set(const std::string& key, Datatype value) {
            // boost::asio::write(socket, boost::asio::buffer());
            // boost::asio::read_until(socket, set_response_buffer, detail::endmarker());
            // return ret;
        }

        Datatype get(const std::string& key) {
            auto request_length = detail::encode_get(key, get_request_buffer);
            boost::asio::write(socket, boost::asio::buffer(get_request_buffer, request_length));
            auto bytes_read = boost::asio::read_until(socket, get_response_buffer, detail::endmarker());
            return detail::decode_get<Datatype>(get_response_buffer, bytes_read);
        }

    private:
        const std::vector<std::string> servers;
        const std::string port;
        boost::asio::io_service service;
        boost::asio::ip::tcp::socket socket;

        std::vector<char> get_request_buffer;
        boost::asio::streambuf get_response_buffer;

        std::vector<char> set_request_buffer;
        boost::asio::streambuf set_response_buffer;
    };
}
