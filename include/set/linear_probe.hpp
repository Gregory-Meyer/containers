#ifndef GREGJM_CONTAINERS_SET_LINEAR_PROBE_HPP
#define GREGJM_CONTAINERS_SET_LINEAR_PROBE_HPP

#include "set/tombstone_bucket.hpp"
#include "utility.hpp"

#include <cstddef>
#include <algorithm>
#include <functional>
#include <iterator>
#include <memory>
#include <type_traits>

#include <gsl/gsl>

namespace gregjm::containers::set {

template <typename T>
class LinearProbe;

template <typename T>
class LinearProbeIterator {
public:
    friend LinearProbe<T>;

    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using reference = const T&;
    using pointer = const T*;
    using iterator_category = std::forward_iterator_tag;

    LinearProbeIterator() = default;

    reference operator*() const {
        return base_->unwrap();
    }

    pointer operator->() const {
        return std::addressof(**this);
    }

    LinearProbeIterator& operator++() {
        ++base_;
        validate();

        return *this;
    }

    LinearProbeIterator operator++(int) {
        const auto to_return = *this;

        ++(*this);

        return to_return;
    }

    friend bool operator==(const LinearProbeIterator &lhs,
                           const LinearProbeIterator &rhs) noexcept {
        return lhs.base_ == rhs.base_;
    }

    friend bool operator!=(const LinearProbeIterator &lhs,
                           const LinearProbeIterator &rhs) noexcept {
        return lhs.base_ != rhs.base_;
    }

private:
    using base_type = typename LinearProbe<T>::view::const_iterator;

    LinearProbeIterator(const base_type current, const base_type end) noexcept
    : base_{ current }, end_{ end } {
        validate();
    }

    void validate() noexcept {
        for (; base_ != end_ && !base_->has_value(); ++base_) { }
    }

    base_type base_;
    base_type end_;
    std::size_t num_occupied_;
};

// linear probing hashing policy
template <typename T>
class LinearProbe {
public:
    using bucket_type = TombstoneBucket<T>;
    using view = gsl::span<bucket_type>;
    using iterator = LinearProbeIterator<T>;
    using const_iterator = iterator;
    using size_type = std::make_unsigned_t<typename view::size_type>;
    using difference_type = std::make_signed_t<size_type>;

    explicit LinearProbe(const view buckets) noexcept
    : buckets_{ buckets } { }

    iterator begin() const noexcept {
        return { buckets_.cbegin(), buckets_.cend() };
    }

    const_iterator cbegin() const noexcept {
        return begin();
    }

    iterator end() const noexcept {
        return { buckets_.cend(), buckets_.cend() };
    }

    const_iterator cend() const noexcept {
        return end();
    }

    std::size_t num_occupied() const noexcept {
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
    const_iterator find(K &&key, const std::size_t hash,
                        E &&eq) const noexcept {
        if (buckets_.empty()) {
            return cend();
        }

        const auto found_iter =
            view_iter_at(find_equal_or_empty(std::forward<K>(key), hash,
                                             std::forward<E>(eq)));

        if (found_iter == buckets_.cend() || found_iter->is_empty()) {
            return cend();
        }

        return make_iterator(found_iter);
    }

    iterator insert(T &&value, const std::size_t hash) {
        if (buckets_.empty()) {
            return end();
        }

        const auto insert_iter = view_iter_at(find_first_valueless(hash));

        if (insert_iter == buckets_.end()) {
            return end();
        }

        insert_iter->emplace(std::move(value));
        ++num_occupied_;
        return make_iterator(insert_iter);
    }

    template <typename K, typename E,
              std::enable_if_t<IS_BINARY_PREDICATE<E, const T&, K>
                              || IS_BINARY_PREDICATE<E, K, const T&>,
                              int> = 0>
    iterator erase(K &&key, const std::size_t hash, E &&eq) {
        if (buckets_.empty()) {
            return end();
        }

        const auto delete_iter =
            view_iter_at(find_equal_or_empty(std::forward<K>(key), hash,
                                             std::forward<E>(eq)));

        if (delete_iter == buckets_.end() || delete_iter->is_empty()) {
            return end();
        }

        delete_iter->set_deleted();
        --num_occupied_;

        return make_iterator(std::next(delete_iter));
    }

    template <typename Hasher,
              std::enable_if_t<IS_HASHER_FOR<Hasher, T>, int> = 0>
    view move_to(Hasher &&hasher, view new_buckets) {
        LinearProbe new_policy{ new_buckets };

        for (auto &bucket : buckets_) {
            if (!bucket.has_value()) {
                continue;
            }

            T &value = bucket.unwrap();
            const std::size_t hash =
                std::invoke(std::forward<Hasher>(hasher), value);

            new_policy.insert(std::move(value), hash);
            bucket.set_deleted();
        }

        adl_swap(buckets_, new_buckets);

        return new_buckets;
    }

    void swap(LinearProbe &other) noexcept {
        adl_swap(buckets_, other.buckets_);
        adl_swap(num_occupied_, other.num_occupied_);
    }

private:
    using ViewIteratorT = typename view::iterator;
    using ViewConstIteratorT = typename view::const_iterator;
    using ViewDifferenceT = typename view::iterator::difference_type;

    static constexpr inline size_type NPOS =
        std::numeric_limits<size_type>::max();

    template <typename K, typename E,
              std::enable_if_t<IS_BINARY_PREDICATE<E, const T&, K>
                              || IS_BINARY_PREDICATE<E, K, const T&>,
                              int> = 0>
    size_type find_equal_or_empty(K &&key, const std::size_t hash,
                                  E &&eq) const {
        const std::size_t start_index = hash % num_buckets();
        const auto start_iter =
            buckets_.cbegin() + static_cast<ViewDifferenceT>(start_index);

        TombstoneEmptyOrPred<K, E> pred{ std::forward<K>(key),
                                         std::forward<E>(eq) };

        const auto first = std::find_if(start_iter, buckets_.cend(), pred);

        if (first != buckets_.cend()) {
            return view_iter_index(first);
        }

        const auto second = std::find_if(buckets_.cbegin(), start_iter, pred);

        if (second == start_iter) {
            return NPOS;
        }

        return view_iter_index(second);
    }

    size_type find_first_valueless(const std::size_t hash) const noexcept {
        const auto is_valueless =
            [](const bucket_type &bucket) { return !bucket.has_value(); };

        const std::size_t start_index = hash % num_buckets();
        const auto start_iter =
            buckets_.cbegin() + static_cast<ViewDifferenceT>(start_index);

        const auto first = std::find_if(start_iter, buckets_.cend(),
                                        is_valueless);

        if (first != buckets_.cend()) {
            return view_iter_index(first);
        }

        const auto second = std::find_if(buckets_.cbegin(), start_iter,
                                         is_valueless);

        if (second == start_iter) {
            return NPOS;
        }

        return view_iter_index(second);
    }

    size_type num_buckets() const noexcept {
        return static_cast<size_type>(buckets_.size());
    }

    iterator make_iterator(const ViewConstIteratorT iter) const noexcept {
        return { iter, buckets_.cend() };
    }

    size_type view_iter_index(const ViewIteratorT iter) noexcept {
        return static_cast<size_type>(std::distance(buckets_.begin(), iter));
    }

    size_type view_iter_index(const ViewConstIteratorT iter) const noexcept {
        return static_cast<size_type>(std::distance(buckets_.cbegin(), iter));
    }

    ViewIteratorT view_iter_at(const size_type index) noexcept {
        if (index == NPOS) {
            return buckets_.end();
        }

        return buckets_.begin() + static_cast<ViewDifferenceT>(index);
    }

    ViewConstIteratorT view_iter_at(const size_type index) const noexcept {
        if (index == NPOS) {
            return buckets_.cend();
        }

        return buckets_.cbegin() + static_cast<ViewDifferenceT>(index);
    }

    view buckets_;
    size_type num_occupied_ = 0;
};

template <typename T>
void swap(LinearProbe<T> &lhs, LinearProbe<T> &rhs) noexcept {
    lhs.swap(rhs);
}
    
} // namespace gregjm::containers::set

#endif
