memcached-cpp
=============

### Intent

The aim of this experiment was to see how good `boost::asio` and `std::future` can work together. As I didn't want to write some theoretical stuff, I tried to apply this to a real application - a memcached client. This is in no way a feature-complete memcached client. 

### How it worked out
Let's start with some code:

    // init your client somewhere
    namespace mc = memcachedcpp;
    mc::async_client<std::string, mc::ip::tcp, mc::protocol::plain> client(servers, port);

    // in some callback handler
    auto res = client.get("somekey");

    // do some other processing
    auto other_str = some_other_foo();

    // wait for result to become ready
    return other_str + res.get();

This is basically how I imagined it to work and the above mentioned code is indeed possible. The question now is whether it's worth it? I personally didn't benchmark anything yet but there is some overhead involved in managing the asynchronous nature. If all you are doing is fetching results, then the synchronous method is probably better.

### How was it done
If you are interested in the actual code, you can find it under "include/memcachedcpp/detail/async_tcp_server.hpp".

On the memcached side we use the `noreply` option on all storing commands to make asynchronous sending possible. This means that no store-command is acknowledged.

The design follows the consumer-producer queue pattern(e.g.: [here][1]) via `boost::asio::io_service::post`.

On sending the actual message format is created and then stored in a buffer which is then passed via `boost::asio::io_service::post` to the internal running thread which runs the callback handler. This requires no additional synchronization beside the one inherit in `post`.

The get command is a little more complicated as it's the only one which waits on a response from the server(s). It's consists of two steps - sending the request and waiting for the result.
When the user calls get a `std::promise` is created whose `std::future` is returned to the user, he can now wait for it to become ready.  
On the server side, the future is bound via a `std::shared_ptr`(if `boost::asio` would support moving on its completion-handlers, the shared_ptr wouldn't be necessary) to the a completion handler which is then `post`ed. The completion handler - executed in the callback handler thread - now moves the promise to a deque which stores all the promises for the current server and sends the actual memcached write command to the server. When the client connects to the server, a `boost::asio::async_read_until` handler is set which now waits for the server to respond with data. 
The data is received in a three-phase approach which was selected to be usable on multi-get(that is currently not implemented, though). In the first phase the message header is read to extract the size of the data package, which is then read and finally the message endmarker, after that it waits for the message header again. In the last phase the received data is set to the promise and the promise is discarded from the internal deque. Every callback sets the async_read for the next.  
All data sharing happens either via `std::future` or via `boost::asio::io_service::post` as such no additional synchronization is needed here. 

Due to the fact that a memcached client may be connected to more than one server at a time, there is internal state for each of these servers such that no data is interleaved as theoretically a callback from different servers could be executed in between a three-phase.

### Open problems
asdfa

### On the memcached side
-hash param
-consistent hasher
-tcp


  [1]: http://www.boost.org/doc/libs/1_53_0/doc/html/boost_asio/example/chat/chat_client.cpp
