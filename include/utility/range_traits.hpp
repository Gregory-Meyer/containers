#ifndef GREGJM_CONTAINERS_UTILITY_RANGE_TRAITS_HPP
#define GREGJM_CONTAINERS_UTILITY_RANGE_TRAITS_HPP

#include "utility/iterator.hpp"
#include "utility/range_utility.hpp"

#include <type_traits>
#include <utility>

namespace gregjm::containers {
inline namespace utility {

template <typename T, typename = void>
struct IsRange : public std::false_type { };

template <typename T>
struct IsRange<
    T,
    std::void_t<decltype(adl_begin(std::declval<T>())),
                decltype(adl_end(std::declval<T>()))>
> : public std::conditional_t<
    IS_ITERATOR<decltype(adl_begin(std::declval<T>()))>,
    std::true_type, std::false_type
> { };

template <typename T>
constexpr inline bool IS_RANGE = IsRange<T>::value;

template <typename T, bool = IS_RANGE<T>>
struct RangeIterator { };

template <typename T>
struct RangeIterator<T, true> {
    using type = decltype(adl_begin(std::declval<T>()));
};

template <typename T>
using RangeIteratorT = typename RangeIterator<T>::type;

} // inline namespace utility
} // namespace gregjm::containers

#endif
