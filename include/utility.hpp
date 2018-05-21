#ifndef GREGJM_CONTAINERS_UTILITY_HPP
#define GREGJM_CONTAINERS_UTILITY_HPP

#include "utility/iterator.hpp"
#include "utility/ranges.hpp"

#include <climits>
#include <iterator>
#include <optional>
#include <type_traits>
#include <utility>

namespace gregjm::containers {

constexpr inline bool is_power_of_2(const unsigned long long x) noexcept {
    // classic Stanford bit hack
    return x > 0 && (x & (x - 1)) == 0;
}

// rounds up to the next power of two, leaves powers of two untouched
// if the next power of two is too large for an ull, returns the maximum value
inline unsigned long long next_power_of_2(const unsigned long long x)
noexcept {
    static constexpr std::size_t ULL_BITS =
        CHAR_BIT * sizeof(unsigned long long);

    if (is_power_of_2(x)) {
        return x;
    }

    const auto leading_zeros = static_cast<std::size_t>(__builtin_clzll(x));

    if (leading_zeros == 0) {
        return std::numeric_limits<unsigned long long>::max();
    }

    const std::size_t occupied_zeros = ULL_BITS - leading_zeros;

    return 1ull << (occupied_zeros);
}

template <typename T>
void adl_swap(T &lhs, T &rhs) noexcept(std::is_nothrow_swappable_v<T>) {
    using std::swap;

    swap(lhs, rhs);
}

// equivalent to std::true_type if T is a hasher for U
template <typename T, typename U>
struct IsHasherFor
: public std::conditional_t<
    std::is_invocable_v<T, const U&>
    && std::is_same_v<std::invoke_result_t<T, const U&>, std::size_t>,
    std::true_type, std::false_type
> { };

// == true if T is a hasher for U
template <typename T, typename U>
constexpr inline bool IS_HASHER_FOR = IsHasherFor<T, U>::value;

} // namespace gregjm::containers

#endif
