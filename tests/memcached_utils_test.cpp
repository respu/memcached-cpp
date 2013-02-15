#include "../include/memcached-cpp/detail/memcached_utils.hpp"
#include "catch.hpp"

#include "boost/asio.hpp"

TEST_CASE("extract_datasize/1", "tests proper extraction of datasize") {
    auto str = "VALUE rofl 0 1234\r\n";
    boost::asio::streambuf test_buf;
    std::ostream os(&test_buf);
    os << str;

    auto supposed = 1234;
    auto test_val = memcachedcpp::detail::extract_datasize(test_buf);
    CHECK(test_val == supposed);
}

TEST_CASE("encode_get/2", "tests encode_get") {
    std::vector<char> buf;
    buf.push_back('X');
    std::string ref = "get some_key \r\n";
    memcachedcpp::detail::encode_get("some_key", buf);
    CHECK(std::equal(buf.begin(), buf.end(), ref.begin()));
}




