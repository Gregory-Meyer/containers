#ifndef GREGJM_CONTAINERS_UTILITY_RANGE_UTILITY_HPP
#define GREGJM_CONTAINERS_UTILITY_RANGE_UTILITY_HPP

#include <iterator>
#include <type_traits>
#include <utility>

namespace gregjm::containers {
inline namespace utility {

template <typename T>
decltype(auto) adl_begin(T &&t) {
    using std::begin;

    return begin(std::forward<T>(t));
}

template <typename T>
decltype(auto) adl_cbegin(T &&t) {
    using std::cbegin;

    return cbegin(std::forward<T>(t));
}

template <typename T>
decltype(auto) adl_end(T &&t) {
    using std::end;

    return end(std::forward<T>(t));
}

template <typename T>
decltype(auto) adl_cend(T &&t) {
    using std::cend;

    return cend(std::forward<T>(t));
}

} // inline namespace utility
} // namespace gregjm::containers

#endif
