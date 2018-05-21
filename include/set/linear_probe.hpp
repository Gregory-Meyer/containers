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
        if (traversed_ < num_occupied_ - 1 && base_ != end_) {
            ++traversed_;
            ++base_;
            validate();
        } else if (traversed_ >= num_occupied_ - 1) {
            base_ = end_;
        }

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
    using UnderlyingT = typename LinearProbe<T>::SpanT::iterator;

    LinearProbeIterator(const UnderlyingT current, const UnderlyingT end,
                        const std::size_t occupied) noexcept
    : base_{ current }, end_{ end },  num_occupied_{ occupied } { }

    void validate() noexcept {
        for (; base_ != end_ && !base_->has_value(); ++base_) { }
    }

    UnderlyingT base_;
    UnderlyingT end_;
    std::size_t traversed_ = 0;
    std::size_t num_occupied_;
};

// linear probing hashing policy
template <typename T>
class LinearProbe {
public:
    using BucketT = TombstoneBucket<T>;
    using SpanT = gsl::span<BucketT>;
    using IteratorT = LinearProbeIterator<T>;

    explicit LinearProbe(const SpanT buckets) noexcept
    : buckets_{ buckets } { }

    const SpanT& buckets() const noexcept {
        return buckets_;
    }

    IteratorT begin() const noexcept {
        return { buckets_.cbegin(), buckets_.cend(), num_occupied() };
    }

    IteratorT cbegin() const noexcept {
        return begin();
    }

    IteratorT end() const noexcept {
        return { buckets_.cend(), buckets_.cend(), num_occupied() };
    }

    IteratorT cend() const noexcept {
        return end();
    }

    std::size_t num_occupied() const noexcept {
        return num_occupied_;
    }

    template <
        typename Key, typename Equal,
        std::enable_if_t<std::is_invocable_v<Equal, Key, const T&>
                         || std::is_invocable_v<Equal, const T&, Key>,
                         int> = 0
    >
    typename SpanT::const_iterator
    find(Key &&key, const std::size_t hash, Equal &&equal) const noexcept {
        if (buckets_.empty()) {
            return buckets_.cend();
        }

        const std::size_t start_index = hash % num_buckets();
        const auto start_iter =
            buckets_.cbegin() + static_cast<SpanDiffT>(start_index);

        const auto is_equal_or_empty = [&key, &equal](const BucketT &bucket) {
            if (bucket.is_empty()) {
                return true;
            } else if (!bucket.has_value()) {
                return false;
            }

            const auto &value = bucket.unwrap();

            if constexpr (std::is_invocable_v<Equal, Key, const T&>) {
                return std::invoke(std::forward<Equal>(equal),
                                   std::forward<Key>(key), value);
            } else {
                return std::invoke(std::forward<Equal>(equal), value,
                                   std::forward<Key>(key));
            }
        };

        const auto first = std::find_if(start_iter, buckets_.cend(),
                                        is_equal_or_empty);

        if (first != buckets_.cend()) {
            if (first->is_empty()) {
                return buckets_.cend();
            }

            return first;
        }

        const auto second = std::find_if(buckets_.cbegin(), start_iter,
                                         is_equal_or_empty);

        if (second == start_iter || second->is_empty()) {
            return buckets_.cend();
        }

        return second;
    }

    typename SpanT::iterator
    insert(T &&value, const std::size_t hash) {
        if (buckets_.empty()) {
            return buckets_.end();
        }

        const auto insert_iter = to_iter(find_insert(hash));

        if (insert_iter == buckets_.end()) {
            return buckets_.end();
        }

        insert_iter->emplace(std::move(value));
        ++num_occupied_;
        return insert_iter;
    }

    template <
        typename Key, typename Equal,
        std::enable_if_t<std::is_invocable_v<Equal, Key, const T&>
                        || std::is_invocable_v<Equal, const T&, Key>,
                        int> = 0
    >
    typename SpanT::iterator
    erase(Key &&key, const std::size_t hash, Equal &&equal) {
        if (buckets_.empty()) {
            return buckets_.end();
        }

        const auto delete_const_iter = find(std::forward<Key>(key), hash,
                                            std::forward<Equal>(equal));
        const auto delete_iter = to_iter(delete_const_iter);

        if (delete_iter == buckets_.end()) {
            return buckets_.end();
        }

        delete_iter->set_deleted();
        --num_occupied_;
        return delete_iter;
    }

    template <typename Hasher,
              std::enable_if_t<IS_HASHER_FOR<Hasher, T>, int> = 0>
    SpanT move_to(Hasher &&hasher, SpanT new_buckets) {
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
    using SpanDiffT = typename SpanT::iterator::difference_type;

    typename SpanT::iterator
    to_iter(const typename SpanT::const_iterator iter) const noexcept {
        const auto index = std::distance(buckets_.cbegin(), iter);

        return buckets_.begin() + index;
    }

    typename SpanT::const_iterator
    find_insert(const std::size_t hash) const noexcept {
        const std::size_t start_index = hash % num_buckets();
        const auto start_iter =
            buckets_.cbegin() + static_cast<SpanDiffT>(start_index);

        const auto is_valueless =
            [](const BucketT &bucket) { return !bucket.has_value(); };

        const auto first = std::find_if(start_iter, buckets_.cend(),
                                        is_valueless);

        if (first != buckets_.cend()) {
            return first;
        }

        const auto second = std::find_if(buckets_.cbegin(), start_iter,
                                         is_valueless);

        if (second == start_iter) {
            return buckets_.cend();
        }

        return second;
    }

    std::size_t num_buckets() const noexcept {
        return static_cast<std::size_t>(buckets_.size());
    }

    SpanT buckets_;
    std::size_t num_occupied_ = 0;
};

template <typename T>
void swap(LinearProbe<T> &lhs, LinearProbe<T> &rhs) noexcept {
    lhs.swap(rhs);
}
    
} // namespace gregjm::containers::set


#endif
