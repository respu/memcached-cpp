// (C) Copyright Stephan Dollberg 2013. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef MEMCACHED_CONSISTENT_HASHER_HPP
#define MEMCACHED_CONSISTENT_HASHER_HPP

#include <boost/range/irange.hpp>

namespace memcachedcpp { namespace detail {

    template<typename hash>
    class consistent_hasher {
    public:
        template<typename server_iter>
        consistent_hasher(server_iter begin, server_iter end) {
            std::vector<std::string> servers(begin, end);
            for(auto&& server_id : boost::irange<std::size_t>(0, servers.size())) {
                for(auto&& i : boost::irange(0, 256)) {
                    consistent_hash[hasher(servers[server_id] + std::to_string(i))] = server_id; 
                }
            }
        }

        std::size_t get_node_id(const typename hash::argument_type& key) {
            auto current_hash = hasher(key);
            auto begin_iter = consistent_hash.upper_bound(current_hash);
            return begin_iter != consistent_hash.end() ? begin_iter->second : consistent_hash.begin()->second;
        }

    private:
        hash hasher;
        std::map<std::size_t, std::size_t> consistent_hash;
    };
}}

#endif // MEMCACHED_CLIENT_HPP
