// (C) Copyright Stephan Dollberg 2013. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef MEMCACHED_CLIENT_HPP
#define MEMCACHED_CLIENT_HPP

#include "detail/memcached_utils.hpp"
#include "config.hpp"
#include "detail/consistent_hasher.hpp"
#include "detail/asio_utils.hpp"

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/asio.hpp>

#include <string>
#include <vector>
#include <utility>
#include <stdexcept>
#include <iostream>
#include <iterator>
#include <tuple>
#include <functional>
#include <unordered_map>

namespace memcachedcpp {

    template<typename Datatype, ip ip_type, protocol protocol_type, typename hasher>
    class client_impl {
    };
 
    template<typename Datatype, typename hasher>
    class client_impl<Datatype, ip::tcp, protocol::plain, hasher> {
    public:
        client_impl(std::vector<std::string> new_servers, std::string port) 
            : servers(std::make_move_iterator(new_servers.begin()), std::make_move_iterator(new_servers.end())), port(std::move(port)), con_hasher(servers.begin(), servers.end())
        {
            connect();
        }

        void set(const std::string& key, const Datatype& value, std::size_t timeout = 0) {
            auto status = store_impl("set", key, value, timeout);
          
            if(status != detail::success_status()) {
                throw std::runtime_error(status);
            }            
        }

        /**
         * @return first indicates whether value was found; second actual data -
         *    default constructed if element was not found
         */
        std::tuple<bool, Datatype> get(const std::string& key) {
            auto server_id = get_server_id(key);
            detail::encode_get(key, write_buffer);
            boost::asio::write(sockets[server_id], boost::asio::buffer(write_buffer, write_buffer.size()));
            
            std::tuple<bool, Datatype> ret{false,{}};

            while(get_one(std::get<1>(ret), server_id)) {
                std::get<0>(ret) = true;
            }

            return ret;
        }

        /**
         * @return std::vector which contains all found elements
         */
        std::vector<Datatype> get_multi(std::vector<std::string> keys) {
            auto server_mapping = get_server_mapping(keys);
            for(auto&& server : server_mapping) {
                auto server_id = std::get<0>(server);
                detail::encode_multi_get(std::get<1>(server), write_buffer);
                boost::asio::write(sockets[server_id], boost::asio::buffer(write_buffer, write_buffer.size()));
            }

            std::vector<Datatype> ret_vector;
            for(auto&& server : server_mapping) {
                Datatype extractor{};
                while(get_one(extractor,std::get<0>(server))) {
                    ret_vector.emplace_back(std::move(extractor));
                }
            }

            return ret_vector;
        } 

        template<typename ...Args>
        std::vector<Datatype> get_multi(Args ...args) {
            std::cout << "variadic" << std::endl;
            std::vector<std::string> keys{ args... };
            return get_multi(std::move(keys));
        }
            


        /**
         * @return true if value was added
         */
        bool add(const std::string& key, const Datatype& value, std::size_t timeout = 0) {
            auto status = store_impl("add", key, value, timeout);
            
            if(status == detail::not_stored_status()) {
                return false; 
            }
            else if(status == detail::success_status()) {
                return true;
            }
            else {
                throw std::runtime_error(status);
            }
        }

        
        /**
         * @return true if value was replaced
         */
        bool replace(const std::string& key, const Datatype& value, std::size_t timeout = 0) {
            auto status = store_impl("replace", key, value, timeout);
            
            if(status == detail::not_stored_status()) {
                return false; 
            }
            else if(status == detail::success_status()) {
                return true;
            }
            else {
                throw std::runtime_error(status);
            }
        }

        /**
         * @return true if value was deleted
         */
        bool del(const std::string& key) {
            auto server_id = get_server_id(key);
            detail::encode_delete(key, write_buffer);
            boost::asio::write(sockets[server_id], boost::asio::buffer(write_buffer, write_buffer.size()));
            auto bytes_read = boost::asio::read_until(sockets[server_id], read_buffer, detail::linefeed());
            auto status = detail::decode_delete(read_buffer, bytes_read);

            return status == detail::deleted();
        }

        /** 
         * @return first indicates whether incr was successful; second is the new value
         */
        std::tuple<bool, Datatype> incr(const std::string& key, Datatype incr_val) {
            return incr_decr_impl("incr", key, incr_val);
        }

        /**
         * @return first indicates whether decr was successful; second is the new value
         */
        std::tuple<bool, Datatype> decr(const std::string& key, Datatype decr_val) {
            return incr_decr_impl("decr", key, decr_val);
        }

    private:
        const std::set<std::string> servers;
        const std::string port;
        detail::consistent_hasher<hasher> con_hasher;
        boost::asio::io_service service;
        boost::ptr_vector<boost::asio::ip::tcp::socket> sockets;

        std::vector<char> write_buffer; 
        boost::asio::streambuf read_buffer; 

        std::tuple<bool, Datatype> incr_decr_impl(const std::string& command, const std::string& key, Datatype incr_val) {
            auto server_id = get_server_id(key);
            detail::encode_incr_decr(command, key, incr_val, write_buffer);
            boost::asio::write(sockets[server_id], boost::asio::buffer(write_buffer, write_buffer.size()));
            boost::asio::read_until(sockets[server_id], read_buffer, detail::linefeed());

            std::tuple<bool, Datatype> ret{false,{}};
            std::get<0>(ret) = detail::decode_incr_decr(read_buffer, std::get<1>(ret));
            
            return ret;
        }

        std::string store_impl(const std::string& command, const std::string& key,
                                 const Datatype& value, std::size_t timeout) {
            auto server_id = get_server_id(key);
            detail::encode_store(command, key, value, timeout, write_buffer);
            boost::asio::write(sockets[server_id], boost::asio::buffer(write_buffer, write_buffer.size()));
            auto bytes_read = boost::asio::read_until(sockets[server_id], read_buffer, detail::linefeed());
            auto status = detail::decode_store(read_buffer, bytes_read);
            return status;
        }

        void connect() {
            detail::connect_n_tcp(sockets, service, servers.begin(), servers.end(), port);
        }

        /**
         * @brief reads one element in a get response from mc-server
         *          reads header line and then extracts data
         * @return false if only END\r\n marker was read, true if element was extracted
         */
        bool get_one(Datatype& data, std::size_t server_id) {
            auto bytes_read = boost::asio::read_until(sockets[server_id], read_buffer, detail::linefeed());

            if(bytes_read == detail::endmarker_length()) { // no results found
                std::istream is(&read_buffer);
                is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                return false;
            }

            auto data_size = detail::extract_datasize(read_buffer);
            auto bytes_left = detail::get_bytes_left(data_size, read_buffer); // check how many bytes we still need to read
            boost::asio::read(sockets[server_id], read_buffer, boost::asio::transfer_at_least(bytes_left));

            detail::decode_get(read_buffer, data_size, data);    
            return true;
        }

        std::size_t get_server_id(const std::string& key) {
            return con_hasher.get_node_id(key);
        }

        /**
         * @brief transform a set of keys into a set of keys for each server
         * @return std::unordered_map<std::size_t, std::vector<std::string>>
         */
        std::unordered_map<std::size_t, std::vector<std::string>> get_server_mapping(std::vector<std::string> keys) {
            std::unordered_map<std::size_t, std::vector<std::string>> ret_map;

            for(auto&& key : keys) {
                auto server_id = con_hasher.get_node_id(key);
                ret_map[server_id].emplace_back(std::move(key));
            }

            return ret_map;
        }


    };

    template<typename T, ip ip_type, protocol prot_type>
    using client = client_impl<T, ip_type, prot_type, std::hash<std::string>>;
}

#endif // MEMCACHED_CLIENT_HPP
