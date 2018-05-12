#ifndef GREGJM_CONTAINERS_ITERATOR_RANGE_HPP
#define GREGJM_CONTAINERS_ITERATOR_RANGE_HPP

#include <type_traits>
#include <utility>

namespace gregjm {
namespace containers {

template <typename Begin, typename End>
class IteratorRange {
public:
    template <typename T, typename U,
              std::enable_if_t<std::is_constructible_v<Begin, T>
                               && std::is_constructible_v<End, U>, int> = 0>
    IteratorRange(T &&begin, U &&end)
    noexcept(std::is_nothrow_constructible_v<Begin, T>
             && std::is_nothrow_constructible_v<End, U>)
    : begin_(std::forward<T>(begin)), end_(std::forward<U>(end)) { }

    const Begin& begin() const noexcept {
        return begin_;
    }

    const End& end() const noexcept {
        return end_;
    }

private:
    Begin begin_{ };
    End end_{ };
};

template <typename Begin, typename End>
IteratorRange<Begin, End> make_iterator_range(Begin first, End last)
noexcept(noexcept(IteratorRange<Begin, End>{ std::move(first),
                                             std::move(last) })) {
    return { std::move(first), std::move(last) };
}

} // namespace containers
} // namespace gregjm

#endif
