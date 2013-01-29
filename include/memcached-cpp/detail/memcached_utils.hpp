// (C) Copyright Stephan Dollberg 2013. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "boost/asio.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/archive/text_iarchive.hpp"
#include "boost/archive/text_oarchive.hpp"
#include <string>
#include <vector>
#include <iterator>

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

    constexpr const char* sucess_status() {
        return "STORED";
    }

    inline std::size_t extract_datasize(boost::asio::streambuf& buffer) {
        std::string str;
        std::istream is(&buffer);
        std::getline(is, str);
        auto data_size_begin_pos = str.rfind(' ') + 1;
        std::string data_size(str.begin() + data_size_begin_pos, str.end() - 1); // -1 for \r
        return boost::lexical_cast<std::size_t>(data_size);
    }

    inline int encode_get(std::string key, std::vector<char>& buffer) {
        constexpr const char* get = "get ";
        auto size = 4 + key.size() + 3;

        buffer.clear();
        buffer.reserve(size);

        std::copy(get, get + 4, std::back_inserter(buffer));
        std::copy(key.begin(), key.end(), std::back_inserter(buffer));
        std::copy(linefeed(), linefeed() + linefeed_length(), std::back_inserter(buffer));

        return size;
    }  


    void decode_get(boost::asio::streambuf& buffer, std::size_t data_size, std::string& output) {
        std::istream is(&buffer);
        output.resize(data_size);
        is.read(&output[0], data_size);
        is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    template<typename Datatype>
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


    inline void encode_store(const char* command, const std::size_t command_length, 
            const std::string& key, const std::string& value, 
            std::size_t timeout, std::vector<char>& buffer) 
    {
        constexpr const char* flags = "0";
        auto timeout_str = boost::lexical_cast<std::string>(timeout);
        auto data_length = boost::lexical_cast<std::string>(value.length());

        buffer.clear();

        std::copy(command, command + command_length, std::back_inserter(buffer));
        buffer.push_back(' ');
        std::copy(key.begin(), key.end(), std::back_inserter(buffer));
        buffer.push_back(' ');
        std::copy(flags, flags + 1, std::back_inserter(buffer));
        buffer.push_back(' ');
        std::copy(timeout_str.begin(), timeout_str.end(), std::back_inserter(buffer));
        buffer.push_back(' ');
        std::copy(data_length.begin(), data_length.end(), std::back_inserter(buffer));
        std::copy(linefeed(), linefeed() + linefeed_length(), std::back_inserter(buffer));
        std::copy(value.begin(), value.end(), std::back_inserter(buffer));
        std::copy(linefeed(), linefeed() + linefeed_length(), std::back_inserter(buffer));
    }


    template<typename Datatype>
    void encode_store(const char* command, const std::size_t command_length, 
            const std::string& key, const Datatype& value, 
            std::size_t timeout, std::vector<char>& buffer) 
    {
        std::ostringstream ss;
        boost::archive::text_oarchive oa(ss);
        oa << value;

        encode_store(command, command_length, key, ss.str(), timeout, buffer);
    }

    inline std::string decode_store(boost::asio::streambuf& buffer, std::size_t bytes_read) {
        std::istream is(&buffer);
        std::string status(bytes_read - linefeed_length(), '\0');
        is.read(&status[0], bytes_read - linefeed_length());
        is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return status;
    }
}}
