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

### What are the benefits
There are two major key benefits. 

The first one being the inherent asynchronous nature. This enables you to do other stuff while waiting for network IO. This is also a great internal boost the calls to the different servers can be fulfilled in parallel. Though, this optimization could also be applied to the non-async client.

The second benefit is that this client is thread-safe out of the box. You can use it from several threads at the same time. This is not possible with the normal client. However there is currently only one internal thread which handles the requests. This can easily be modified to use more threads.

### Open problems
A currently unhandled problem is the treatment of errors. In the current implementation the client just stops its async runner thread when error occurs which leaves the client in an unusable state.

The problem is that errors might occur during reading and writing data. If the error happens on write, there is no way the client can tell that the user as a storage command is basically a fire and forget task. This could be solved by setting an invalid flag which gets checked on every call call.  
On the reader side we have the possibility to set the error as an exception to the future. 

The problem now rises as how to coordinate the reads with the writes and with further ones? Should the client automatically try to reconnect? Should only the latest promise get the exception or should all promises receive it?

This needs some further research.

### On the memcached side
Lastly, I want to show which features the client supports out of the memcached feature set.

The client currently only supports tcp on the plain text protocol. The normal client supports all memcached operations beside the "cas" stuff. The async-client currently only supports set and get.

You can write your own specific hash-function and pass it as a template parameter:

    struct my_hash {
        std::size_t operator()(const std::string& t) const {
            return t.size(); // dumb
        }
    };

    namespace mc = memcachedcpp;
    mc::async_client<std::string, mc::ip::tcp, mc::protocol::plain, my_hash> client(servers, port);

To manage the case of a server change, a consistent hasher is used which tries to keep the server changes low.



-hash param
-consistent hasher
-tcp


  [1]: http://www.boost.org/doc/libs/1_53_0/doc/html/boost_asio/example/chat/chat_client.cpp
