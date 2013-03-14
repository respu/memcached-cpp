// (C) Copyright Stephan Dollberg 2013. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef MEMCACHEDCPP_MEMCACHED_UTILS_HPP
#define MEMCACHEDCPP_MEMCACHED_UTILS_HPP

#include "message_creator.hpp"

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/range/algorithm_ext.hpp>
#include <boost/range/algorithm.hpp>

#include <string>
#include <vector>
#include <iterator>
#include <type_traits>

namespace memcachedcpp { namespace detail {

    constexpr const char* linefeed() {
        return "\r\n";
    }

    constexpr std::size_t linefeed_length() {
        return 2;
    }

    constexpr const char* endmarker() {
        return "END\r\n";
    }

    constexpr std::size_t endmarker_length() {
        return 5;
    }

    constexpr const char* success_status() {
        return "STORED";
    }

    constexpr const char* not_stored_status() {
        return "NOT_STORED";
    }

    constexpr const char* not_found_status() {
        return "NOT_FOUND";
    }
   
    constexpr const char* deleted() {
        return "DELETED";
    }

    constexpr const char* noreply() {
        return "noreply";
    }

    /**
     * @brief reads header line from get response and returns size of data
     *          removes complete header from buffer
     * @return data_size without ending linefeed
     */
    inline std::size_t extract_datasize(boost::asio::streambuf& buffer) {
        std::string str;
        std::istream is(&buffer);
        std::getline(is, str);
        /* note that this should actually find the second to last whitespace 
         *  but mc server in contrast to what is specified in the 
         * protocol file doesn't add whitespace before \r\n */
        auto data_size_begin_pos = str.rfind(' ') + 1;
        std::string data_size(str.begin() + data_size_begin_pos, str.end() - 1); // -1 for \r
        return boost::lexical_cast<std::size_t>(data_size);
    }

    inline int encode_get(const std::string& key, std::vector<char>& buffer) {
        buffer.clear();
        fill_buffer(buffer, "get", key, linefeed());
        return buffer.size();
    }  

    inline void encode_multi_get(const std::vector<std::string>& keys, std::vector<char>& buffer) {
        buffer.clear();
        fill_buffer(buffer, "get");
        buffer.push_back(' ');
        std::for_each(keys.begin(), keys.end(), [&] (const std::string& x) { boost::push_back(buffer, x); buffer.push_back(' '); });
        fill_buffer(buffer, linefeed());
    }


    void decode_get(boost::asio::streambuf& buffer, std::size_t data_size, std::string& output) {
        std::istream is(&buffer);
        output.resize(data_size);
        is.read(&output[0], data_size);
        is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    template<typename Datatype, EnableIf<std::is_integral<Datatype>> = _>
    void decode_get(boost::asio::streambuf& buffer, std::size_t bytes, Datatype& output) {
        std::string str;
        decode_get(buffer, bytes, str);
        output = boost::lexical_cast<Datatype>(str);
    }

    template<typename Datatype, DisableIf<std::is_integral<Datatype>> = _>
    void decode_get(boost::asio::streambuf& buffer, std::size_t bytes, Datatype& output) {
        std::string raw_data;
        decode_get(buffer, bytes, raw_data);

        if(raw_data.empty()) {
            return;
        }

        std::stringstream ss(raw_data);
        boost::archive::text_iarchive ia(ss);
        ia >> output;
    }

    inline void encode_store(const std::string& command, 
            const std::string& key, const std::string& value, 
            std::size_t timeout, std::vector<char>& buffer) 
    {
        auto timeout_str = boost::lexical_cast<std::string>(timeout);
        auto data_length = boost::lexical_cast<std::string>(value.length());

        buffer.clear();
        fill_buffer(buffer, command, key, "0", timeout_str, data_length, linefeed());
        fill_buffer(buffer, value);
        fill_buffer(buffer, linefeed());
    }

    template<typename Datatype, EnableIf<std::is_integral<Datatype>> = _>
    void encode_store(const std::string& command,
            const std::string& key, const Datatype& value,
            std::size_t timeout, std::vector<char>& buffer) 
    {
        auto stringified = boost::lexical_cast<std::string>(value);
        encode_store(command, key, stringified, timeout, buffer);
    }

    template<typename Datatype, DisableIf<std::is_integral<Datatype>> = _>
    void encode_store(const std::string& command,
            const std::string& key, const Datatype& value, 
            std::size_t timeout, std::vector<char>& buffer) 
    {
        std::ostringstream ss;
        boost::archive::text_oarchive oa(ss);
        oa << value;

        encode_store(command, key, ss.str(), timeout, buffer);
    }

    inline std::string decode_store(boost::asio::streambuf& buffer, std::size_t bytes_read) {
        std::istream is(&buffer);
        std::string status(bytes_read - linefeed_length(), '\0');
        is.read(&status[0], bytes_read - linefeed_length());
        is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return status;
    }

    inline void encode_delete(const std::string& key, std::vector<char>& buffer) {
        buffer.clear();
        fill_buffer(buffer, "delete", key, linefeed());
    }

    inline std::string decode_delete(boost::asio::streambuf& buffer, std::size_t bytes_read) {
        return decode_store(buffer, bytes_read);
    }
    
    template<typename Datatype>
    void encode_incr_decr(const std::string& command, const std::string& key, Datatype value, std::vector<char>& buffer) {
        std::string stringified = boost::lexical_cast<std::string>(value);
        buffer.clear();
        fill_buffer(buffer, command, key, stringified, linefeed());
    }

    template<typename Datatype>
    bool decode_incr_decr(boost::asio::streambuf& buffer, Datatype& output) {
        std::istream is(&buffer);
        std::string destringifier;
        std::getline(is, destringifier);
        destringifier.pop_back(); // pop back \r

        if(destringifier == not_found_status()) {
            return false;
        }

        output = boost::lexical_cast<Datatype>(destringifier);
        return true; 
    }


    std::size_t get_bytes_left(std::size_t data_size, const boost::asio::streambuf& read_buffer) {
        if(data_size > read_buffer.size()) {
            return data_size - read_buffer.size();
        }
        else {
            return 0;
        }
    }
 
}}

#endif // MEMCACHEDCPP_MEMCACHED_UTILS_HPP
