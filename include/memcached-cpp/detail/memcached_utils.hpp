// (C) Copyright Stephan Dollberg 2013. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "boost/asio.hpp"
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

    inline int encode_get(std::string key, std::vector<char>& buffer) {
        static const std::string get = "get ";
        static const std::string end = " \r\n";
        auto size = get.size() + key.size() + end.size();

        buffer.clear();
        buffer.reserve(size);

        std::copy(get.begin(), get.end(), std::back_inserter(buffer));
        std::copy(key.begin(), key.end(), std::back_inserter(buffer));
        std::copy(end.begin(), end.end(), std::back_inserter(buffer));
        return size;
    }  

    template<typename Datatype, typename std::enable_if<std::is_same<std::string, Datatype>::value, int>::type = 0>
    Datatype decode_get(boost::asio::streambuf& buffer, std::size_t bytes) {
        std::istream is(&buffer);
        std::string data(bytes - endmarker_length(), '\0');
        std::getline(is, data);
        /* from bytes we subtract:
        * - headersize: data.size()
        * - 1: for \n after header which is not in data.size()
        * - linefeed_length: linefeed after data block
        * - endmarker_length: length of endmarker
        */
        auto data_size = bytes - data.size() - 1 - linefeed_length() - endmarker_length();
        is.read(&data[0], data_size);
        is.seekg(endmarker_length(), std::ios_base::cur);
        data.resize(data_size);
        return data;
    }
}}
