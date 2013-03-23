// (C) Copyright Stephan Dollberg 2013. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef MEMCACHED_CONSISTENT_HASHER_HPP
#define MEMCACHED_CONSISTENT_HASHER_HPP

#include <boost/range/irange.hpp>
#include <map>

namespace memcachedcpp { namespace detail {

    template<typename hash>
    class consistent_hasher {
    public:
        template<typename server_iter>
        consistent_hasher(server_iter begin, server_iter end) {
            auto current_server_id = 0;
            for(auto&& server_name : boost::iterator_range<server_iter>(begin, end)) {
                for(int i : boost::irange(0, 256)) {
                    consistent_hash[hasher(server_name + std::to_string(i))] = current_server_id; 
                }
                current_server_id++;
            }
        }

        std::size_t get_node_id(const typename hash::argument_type& key) const {
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
