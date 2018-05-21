#ifndef GREGJM_CONTAINERS_SET_LINEAR_PROBE_HPP
#define GREGJM_CONTAINERS_SET_LINEAR_PROBE_HPP

#include "set/tombstone_bucket.hpp"

#include <algorithm>
#include <functional>
#include <iterator>
#include <type_traits>

#include <gsl/gsl>

namespace gregjm::containers::set {

// linear probing hashing policy
template <typename Value>
class LinearProbe {
public:
    using BucketT = TombstoneBucket<Value>;
    using SpanT = gsl::span<BucketT>;

    explicit LinearProbe(const SpanT buckets) noexcept
    : buckets_{ buckets } { }

    const SpanT& buckets() const noexcept {
        return buckets_;
    }

    template <
        typename Key, typename Equal,
        std::enable_if_t<std::is_invocable_v<Equal, Key, const Value&>
                         || std::is_invocable_v<Equal, const Value&, Key>,
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

            const auto &value = bucket.get_value().value().get();

            if constexpr (std::is_invocable_v<Equal, Key, const Value&>) {
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
    insert(Value &&value, const std::size_t hash) const {
        if (buckets_.empty()) {
            return buckets_.end();
        }

        const auto insert_iter = to_iter(find_insert(hash));

        if (insert_iter == buckets_.end()) {
            return buckets_.end();
        }

        insert_iter->emplace(std::move(value));
        return insert_iter;
    }

    template <
        typename Key, typename Equal,
        std::enable_if_t<
            std::is_invocable_v<Equal, Key, const Value&>
            || std::is_invocable_v<Equal, const Value&, Key>,
            int
        > = 0
    >
    typename SpanT::iterator
    erase(Key &&key, const std::size_t hash, Equal &&equal) const {
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
        return delete_iter;
    }

    template <typename Hasher>
    SpanT move_to(Hasher &&hasher, SpanT new_buckets) {
        LinearProbe new_policy(new_buckets);

        for (auto &bucket : buckets_) {
            if (!bucket.has_value()) {
                continue;
            }

            Value &value = bucket.get_value().value();
            const std::size_t hash =
                std::invoke(std::forward<Hasher>(hasher), value);

            new_policy.insert(std::move(value), hash);
            bucket.set_deleted();
        }

        {
            using std::swap;
            swap(buckets_, new_buckets);
        }

        return new_buckets;
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
};
    
} // namespace gregjm::containers::set


#endif
