#include "../include/memcached-cpp/detail/memcached_utils.hpp"
#include "catch.hpp"

#include "boost/asio.hpp"

template<typename T>
std::string lc(T&& t) {
    return boost::lexical_cast<std::string>(std::forward<T>(t));
}

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

TEST_CASE("decode_get/3", "tests decode_get with string") {
    std::string output;

    std::string ref = "randomevalue123\r\n";
    boost::asio::streambuf sbuf;
    std::ostream os(&sbuf);
    os << ref;

    memcachedcpp::detail::decode_get(sbuf, ref.size() - 2, output);
    CHECK(std::equal(output.begin(), output.end(), ref.begin()));
    CHECK(sbuf.size() == 0);
}

TEST_CASE("decode_get/4", "tests decode_get with int") {
    int output = 0;

    std::string ref = "42\r\n";
    boost::asio::streambuf sbuf;
    std::ostream os(&sbuf);
    os << ref;

    memcachedcpp::detail::decode_get(sbuf, ref.size() - 2, output);
    CHECK(output == 42);
    CHECK(sbuf.size() == 0);
}

TEST_CASE("decode_get/5", "tests decode_get with boost serializeable target") {
    // TODO 
}

TEST_CASE("encode_set/6", "tests encode_store-set with std::string") {
    using boost::lexical_cast;
    std::string key = "somekey";
    std::string value = "randomvalue1234";
    std::size_t timeout = 12;
    std::vector<char> buf;
    std::string ref = 
        "set " + key + " 0 " + lc(timeout) + " " + lc(value.size()) + " \r\n" 
          + value + "\r\n";
    memcachedcpp::detail::encode_store("set", key, value, timeout, buf);
    CHECK(std::equal(buf.begin(), buf.end(), ref.begin()));
}

TEST_CASE("encode_set/7", "tests encode_store-set with int") {
    std::string key = "somekey";
    std::string str_value = "42";
    int i_value = 42;
    std::size_t timeout = 12;
    std::vector<char> buf;
    std::string ref = "set " + key + " 0 " + lc(timeout) + " " + lc(str_value.size())
        + " \r\n" + str_value + "\r\n";
    memcachedcpp::detail::encode_store("set", key, i_value, timeout, buf);
    CHECK(std::equal(buf.begin(), buf.end(), ref.begin()));
}

TEST_CASE("encode_set/8", "tests encode_store-set with serializeable class") {
    // TODO
}
