#ifndef GREGJM_CONTAINERS_SET_ROBIN_HOOD_HPP
#define GREGJM_CONTAINERS_SET_ROBIN_HOOD_HPP

#include "set/dib_bucket.hpp"
#include "utility.hpp"

#include <cstddef>
#include <optional>
#include <type_traits>
#include <utility>

#include <gsl/gsl>

namespace gregjm::containers::set {

template <typename T>
class RobinHood;

template <typename T>
class RobinHoodIterator {
public:
    friend RobinHood<T>;

    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using reference = const T&;
    using pointer = const T*;
    using iterator_category = std::forward_iterator_tag;

    RobinHoodIterator() = default;

    [[nodiscard]] reference operator*() const {
        return current_->unwrap();
    }

    [[nodiscard]] pointer operator->() const {
        return std::addressof(**this);
    }

    RobinHoodIterator& operator++() {
        if (current_ != end_) {
            ++current_;
            validate();
        }

        return *this;
    }

    RobinHoodIterator operator++(int) {
        const auto to_return = *this;

        ++(*this);

        return to_return;
    }

    [[nodiscard]] friend bool operator==(
        const RobinHoodIterator &lhs, const RobinHoodIterator &rhs
    ) noexcept {
        return lhs.base_ == rhs.base_;
    }

    [[nodiscard]] friend bool operator!=(
        const RobinHoodIterator &lhs, const RobinHoodIterator &rhs
    ) noexcept {
        return lhs.base_ != rhs.base_;
    }

private:
    using BaseT = typename RobinHood<T>::view::iterator;

    RobinHoodIterator(BaseT current, BaseT last) noexcept
    : current_{ std::move(current) }, end_{ std::move(last) } {
        validate();
    }

    void validate() noexcept {
        for (; current_ != end_ && !current_->has_value(); ++current_) { }
    }

    BaseT current_;
    BaseT end_;
};

template <typename T>
class RobinHood {
public:
    using bucket_type = DibBucket<T>;
    using view = gsl::span<bucket_type>;
    using iterator = RobinHoodIterator<T>;
    using const_iterator = iterator;
    using size_type = std::make_unsigned_t<typename view::size_type>;
    using difference_type = std::make_signed_t<size_type>;

    explicit RobinHood(const view buckets) noexcept : buckets_{ buckets } { }

    [[nodiscard]] iterator begin() const noexcept {
        return { buckets_.cbegin(), buckets_.cend() };
    }

    [[nodiscard]] const_iterator cbegin() const noexcept {
        return begin();
    }

    [[nodiscard]] iterator end() const noexcept {
        return { buckets_.cend(), buckets_.cend() };
    }

    [[nodiscard]] const_iterator cend() const noexcept {
        return end();
    }

    [[nodiscard]] size_type num_occupied() const noexcept {
        return num_occupied_;
    }

    void clear() noexcept {
        for (auto &bucket : buckets_) {
            bucket.set_empty();
        }

        num_occupied_ = 0;
    }

    template <typename K, typename E,
              std::enable_if_t<IS_BINARY_PREDICATE<E, const T&, K>
                              || IS_BINARY_PREDICATE<E, K, const T&>,
                              int> = 0>
    [[nodiscard]] const_iterator find(K &&key, const std::size_t hash,
                                      E &&eq) const noexcept;

    iterator insert(T &&value, const std::size_t hash);

    template <typename K, typename E,
              std::enable_if_t<IS_BINARY_PREDICATE<E, const T&, K>
                              || IS_BINARY_PREDICATE<E, K, const T&>,
                              int> = 0>
    iterator erase(K &&key, const std::size_t hash, E &&eq);

    template <typename Hasher,
              std::enable_if_t<IS_HASHER_FOR<Hasher, T>, int> = 0>
    view move_to(Hasher &&hasher, view new_buckets);

    void swap(RobinHood &other) noexcept;

private:
    std::pair<size_type, std::optional<size_type>>
    find_insert_swap_indices(const std::size_t hash) {
        const auto start_index = static_cast<size_type>(hash % num_buckets());
    }

    size_type num_buckets() const noexcept {
        return static_cast<size_type>(buckets_.size());
    }

    view buckets_;
    size_type num_occupied_ = 0;
};

template <typename T>
void swap(RobinHood<T> &lhs, RobinHood<T> &rhs) noexcept;

} // namespace gregjm::containers::set

#endif
