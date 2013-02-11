#include "catch.hpp"
#include "../include/memcached-cpp/detail/message_creator.hpp"
#include <vector>
#include <algorithm>

// tests for fill_buffer util function

TEST_CASE("fill_buffer/1", "tests basic usage1") {
    std::vector<char> v;
    memcachedcpp::detail::fill_buffer(v, "hello", "world", "123");
    std::string checker = "hello world 123";
    CHECK(std::equal(v.begin(), v.end(), checker.begin())); 

}
