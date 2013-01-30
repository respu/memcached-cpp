// (C) Copyright Stephan Dollberg 2013. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "detail/memcached_utils.hpp"

#include "boost/asio.hpp"
#include <string>
#include <vector>
#include <utility>
#include <stdexcept>
#include <iostream>
#include <iterator>
#include <tuple>

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

        void set(const std::string& key, const Datatype& value, std::size_t timeout = 0) {
            detail::encode_store("set", 3, key, value, timeout, set_request_buffer);
            boost::asio::write(socket, boost::asio::buffer(set_request_buffer, set_request_buffer.size()));
            auto bytes_read = boost::asio::read_until(socket, set_response_buffer, detail::linefeed());
            auto status = detail::decode_store(set_response_buffer, bytes_read);

            if(status != detail::sucess_status()) {
                throw std::runtime_error(status);
            }            
        }

        std::tuple<bool, Datatype> get(const std::string& key) {
            auto request_length = detail::encode_get(key, get_request_buffer);
            boost::asio::write(socket, boost::asio::buffer(get_request_buffer, request_length));
            
            std::tuple<bool, Datatype> ret{false,{}};
            
            while(get_one(std::get<1>(ret))) {
                std::get<0>(ret) = true;
            }

            return ret;
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

        bool get_one(Datatype& data) {
            auto bytes_read = boost::asio::read_until(socket, get_response_buffer, detail::linefeed());

            if(bytes_read == detail::endmarker_length()) { // no results found
                std::istream is(&get_response_buffer);
                is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                return false;
            }

            auto data_size = detail::extract_datasize(get_response_buffer);

            boost::asio::read(socket, get_response_buffer, 
                [&] (const boost::system::error_code&, std::size_t bytes_transfered) -> bool {
                    return bytes_transfered <= data_size;
                });
                
            detail::decode_get(get_response_buffer, data_size, data);    
            return true;
        }
    };
}
