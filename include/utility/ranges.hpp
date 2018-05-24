#ifndef GREGJM_CONTAINERS_UTILITY_RANGES_HPP
#define GREGJM_CONTAINERS_UTILITY_RANGES_HPP

#include "utility/iterator.hpp"
#include "utility/range_traits.hpp"
#include "utility/range_utility.hpp"

#include <functional>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>

namespace gregjm::containers {
inline namespace utility {

template <typename I, std::enable_if_t<IS_ITERATOR<I>, int> = 0>
class Range {
    static constexpr inline bool HAS_SIZE = IS_FORWARD_ITERATOR<I>;

public:
    using difference_type = IteratorDifferenceTypeT<I>;
    using size_type = std::make_unsigned_t<difference_type>;

    constexpr Range(I first, I last)
    noexcept(std::is_nothrow_move_constructible_v<I>)
    : begin_{ std::move(first) }, end_{ std::move(last) } { }

    constexpr I begin() const
    noexcept(std::is_nothrow_copy_constructible_v<I>) {
        return begin_;
    }

    constexpr I end() const noexcept(std::is_nothrow_copy_constructible_v<I>) {
        return end_;
    }

    constexpr bool has_size() const noexcept {
        return HAS_SIZE;
    }

    constexpr size_type size() const noexcept {
        if constexpr (!HAS_SIZE) {
            return 0;
        } else {
            return static_cast<size_type>(std::distance(begin(), end()));
        }
    }

private:
    I begin_;
    I end_;
};

template <typename I, std::enable_if_t<IS_ITERATOR<I>, int> = 0>
constexpr Range<I> make_range(I first, I last)
noexcept(std::is_nothrow_move_constructible_v<I>) {
    return { std::move(first), std::move(last) };
}

template <typename R, std::enable_if_t<IS_RANGE<R>, int> = 0>
constexpr Range<RangeIteratorT<R>> make_range(R &&range)
noexcept(noexcept(make_range(adl_begin(std::forward<R>(range)),
                             adl_end(std::forward<R>(range))))) {
    return make_range(adl_begin(std::forward<R>(range)),
                      adl_end(std::forward<R>(range)));
}

template <typename I, typename P,
          std::enable_if_t<
              IS_INPUT_ITERATOR<I>
              && std::is_invocable_v<const P&, IteratorReferenceT<I>>,
              int
          > = 0>
class FilterIterator {
public:
    static constexpr inline bool IS_FORWARD_ITERATOR =
        std::is_default_constructible_v<I>
        && std::is_default_constructible_v<P>
        && gregjm::containers::utility::IS_FORWARD_ITERATOR<I>
        && (std::is_same_v<IteratorValueTypeT<I>&, IteratorReferenceT<I>>
            || std::is_same_v<const IteratorValueTypeT<I>&,
                              IteratorReferenceT<I>>);

    using value_type = IteratorValueTypeT<I>;
    using difference_type = IteratorDifferenceTypeT<I>;
    using reference = IteratorReferenceT<I>;
    using pointer = IteratorPointerT<I>;
    using iterator_category = std::conditional_t<IS_FORWARD_ITERATOR, std::forward_iterator_tag, std::input_iterator_tag>;

    FilterIterator() = default;

    constexpr FilterIterator(I current, I end, P predicate)
    : base_{ std::move(current) }, end_{ std::move(end) },
      pred_{ std::move(predicate) } {
        validate();
    }

    constexpr reference operator*() const {
        return *base_;
    }

    constexpr pointer operator->() const {
        return static_cast<IteratorPointerT<I>>(std::addressof(*base_));
    }

    constexpr FilterIterator& operator++() {
        if (base_ != end_) {
            ++base_;
            validate();          
        }

        return *this;
    }

    constexpr FilterIterator operator++(int) {
        const auto to_return = *this;

        ++(*this);

        return to_return;
    }

    friend constexpr bool operator==(const FilterIterator &lhs,
                                     const FilterIterator &rhs) {
        return lhs.base_ == rhs.base_;
    }

    friend constexpr bool operator!=(const FilterIterator &lhs,
                                     const FilterIterator &rhs) {
        return lhs.base_ != rhs.base_;
    }

private:
    constexpr void validate() {
        for (; base_ != end_ && !std::invoke(pred_, *base_) ; ++base_) { }
    }

    I base_;
    I end_;
    P pred_;
};

template <typename I, typename P,
          std::enable_if_t<
              IS_INPUT_ITERATOR<I>
              && std::is_invocable_v<const P&, IteratorReferenceT<I>>,
              int
          > = 0>
struct Filter {
    static constexpr inline bool HAS_SIZE = IS_FORWARD_ITERATOR<I>;

public:
    using difference_type = IteratorDifferenceTypeT<I>;
    using size_type = std::make_unsigned_t<difference_type>;

    constexpr Filter(I first, I last, P predicate)
    : begin_{ std::move(first) }, end_{ std::move(last) },
      pred_{ std::move(predicate) } { }

    constexpr FilterIterator<I, P> begin() const {
        return { begin_, end_, pred_ };
    }

    constexpr FilterIterator<I, P> end() const {
        return { end_, end_, pred_ };
    }

    constexpr bool has_size() const noexcept {
        return HAS_SIZE;
    }

    constexpr size_type size() const noexcept {
        if constexpr (!HAS_SIZE) {
            return 0;
        } else {
            return static_cast<size_type>(std::distance(begin(), end()));
        }
    }

private:
    I begin_;
    I end_;
    P pred_;
};

template <typename I, typename P,
          std::enable_if_t<
              IS_INPUT_ITERATOR<I>
              && std::is_invocable_v<const P&, IteratorReferenceT<I>>,
              int
          > = 0>
Filter<I, P> make_filter(I first, I last, P predicate) {
    return { std::move(first), std::move(last), std::move(predicate) };
}

template <typename R, typename P,
          std::enable_if_t<
              IS_RANGE<R>
              && std::is_invocable_v<const P&,
                                     IteratorReferenceT<RangeIteratorT<R>>>,
              int
          > = 0>
Filter<RangeIteratorT<R>, P> make_filter(R &&range, P predicate) {
    return make_filter(std::begin(std::forward<R>(range)),
                       std::end(std::forward<R>(range)),
                       std::move(predicate));
}

template <typename I, typename T = std::size_t,
          std::enable_if_t<IS_ITERATOR<I>, int> = 0>
class EnumerateIterator {
public:
    static constexpr inline bool IS_FORWARD_ITERATOR =
        std::is_default_constructible_v<I>
        && gregjm::containers::utility::IS_FORWARD_ITERATOR<I>
        && (std::is_same_v<IteratorValueTypeT<I>&, IteratorReferenceT<I>>
            || std::is_same_v<const IteratorValueTypeT<I>&,
                              IteratorReferenceT<I>>);

    using value_type = std::pair<T, IteratorValueTypeT<I>>;
    using difference_type = IteratorDifferenceTypeT<I>;
    using reference = std::pair<T, IteratorReferenceT<I>>;
    using pointer = std::pair<T, IteratorPointerT<I>>;
    using iterator_category = std::conditional_t<IS_FORWARD_ITERATOR, std::forward_iterator_tag, std::input_iterator_tag>;

    explicit EnumerateIterator(I current) noexcept
    : current_{ std::move(current) } { }

    EnumerateIterator(I current, T start_index, T step = 1) noexcept
    : current_{ std::move(current) }, index_{ std::move(start_index) },
      step_{ std::move(step) } { }

    constexpr reference operator*() const {
        return { index_, *current_ };
    }

    constexpr EnumerateIterator& operator++() {
        ++current_;
        index_ += step_;

        return *this;
    }

    constexpr EnumerateIterator operator++(int) {
        const auto to_return = *this;

        ++(*this);

        return to_return;
    }

    friend constexpr bool operator==(const EnumerateIterator &lhs,
                                     const EnumerateIterator &rhs) {
        return lhs.base_ == rhs.base_;
    }

    friend constexpr bool operator!=(const EnumerateIterator &lhs,
                                     const EnumerateIterator &rhs) {
        return lhs.base_ != rhs.base_;
    }

private:
    I current_;
    T index_ = 0;
    T step_ = 1;
};

template <typename I, typename T = std::size_t,
          std::enable_if_t<IS_INPUT_ITERATOR<I>, int> = 0>
struct Enumerate {
    static constexpr inline bool HAS_SIZE = IS_FORWARD_ITERATOR<I>;

public:
    using difference_type = IteratorDifferenceTypeT<I>;
    using size_type = std::make_unsigned_t<difference_type>;

    constexpr Enumerate(I first, I last, T start = 0, T step = 1)
    : begin_{ std::move(first) }, end_{ std::move(last) },
      start_{ std::move(start) }, step_{ std::move(step) } { }

    constexpr EnumerateIterator<I, T> begin() const {
        return { begin_, start_, step_ };
    }

    constexpr EnumerateIterator<I, T> end() const {
        return { end_, start_, step_ };
    }

    constexpr bool has_size() const noexcept {
        return HAS_SIZE;
    }

    constexpr size_type size() const noexcept {
        if constexpr (!HAS_SIZE) {
            return 0;
        } else {
            return static_cast<size_type>(std::distance(begin(), end()));
        }
    }

private:
    I begin_;
    I end_;
    T start_ = 0;
    T step_ = 1;
};

template <typename T = std::size_t, typename I,
          std::enable_if_t<IS_INPUT_ITERATOR<I>, int> = 0>
Enumerate<I, T> make_enumerate(I first, I last, T start = 0, T step = 1) {
    return { std::move(first), std::move(last),
             std::move(start), std::move(step) };
}

template <typename T = std::size_t, typename R,
          std::enable_if_t<IS_RANGE<R>, int> = 0>
Filter<RangeIteratorT<R>, T> make_enumerate(R &&range, T start = 0,
                                            T step = 1) {
    return make_filter(std::begin(std::forward<R>(range)),
                       std::end(std::forward<R>(range)),
                       std::move(start), std::move(step));
}

} // inline namespace utility
} // namespace gregjm::containers

#endif
