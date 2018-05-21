#ifndef GREGJM_CONTAINERS_UTILITY_HPP
#define GREGJM_CONTAINERS_UTILITY_HPP

#include <climits>
#include <iterator>
#include <optional>
#include <type_traits>

namespace gregjm::containers {

template <
    typename I,
    std::enable_if_t<
        std::is_same_v<typename std::iterator_traits<I>::iterator_category,
                       std::input_iterator_tag>, int
    > = 0
>
std::optional<std::size_t> range_size(I, I) noexcept {
    return std::nullopt;
}

template <
    typename I,
    std::enable_if_t<
        std::is_same_v<typename std::iterator_traits<I>::iterator_category,
                       std::output_iterator_tag>, int
    > = 0
>
std::optional<std::size_t> range_size(I, I) noexcept {
    return std::nullopt;
}

template <
    typename I,
    std::enable_if_t<
        std::is_base_of_v<std::forward_iterator_tag,
                          typename std::iterator_traits<I>::iterator_category>,
        int
    > = 0
>
std::optional<std::size_t> range_size(const I first, const I last) noexcept {
    return { static_cast<std::size_t>(std::distance(first, last)) };
}

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

} // namespace gregjm::containers

#endif
