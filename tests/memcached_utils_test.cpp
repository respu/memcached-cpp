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




