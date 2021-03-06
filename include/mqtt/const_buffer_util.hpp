// Copyright Takatoshi Kondo 2018
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#if !defined(MQTT_CONST_BUFFER_UTIL_HPP)
#define MQTT_CONST_BUFFER_UTIL_HPP

#include <boost/asio/buffer.hpp>

namespace mqtt {

namespace as = boost::asio;

inline char const* get_pointer(as::const_buffer const& cb) {
#if BOOST_VERION >= 106600
    return static_cast<char const*>(cb.data());
#else  // BOOST_VERION >= 106600
    return as::buffer_cast<char const*>(cb);
#endif // BOOST_VERION >= 106600
}

inline std::size_t get_size(as::const_buffer const& cb) {
#if BOOST_VERION >= 106600
    return cb.size();
#else  // BOOST_VERION >= 106600
    return as::buffer_size(cb);
#endif // BOOST_VERION >= 106600
}

} // namespace mqtt

#endif // MQTT_CONST_BUFFER_UTIL_HPP
