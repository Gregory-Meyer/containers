#ifndef GREGJM_CONTAINERS_SET_LINEAR_PROBE_HPP
#define GREGJM_CONTAINERS_SET_LINEAR_PROBE_HPP

#include "set/bucket.hpp"

#include <functional>
#include <type_traits>

#include <gsl/gsl>

namespace gregjm::containers::set {

// linear probing hashing policy
template <typename Value>
class LinearProbe {
public:
    using BucketSpanT = gsl::span<Bucket<Value>>;
    using BucketSpanIterT = typename BucketSpanT::iterator;

    static BucketSpanIterT insert(const BucketSpanT buckets, Value &&value,
                                  const std::size_t hash)
    noexcept(std::is_nothrow_move_constructible_v<Value>) {
        const std::size_t start_index = hash % buckets.size();
        const auto start_iter = buckets.begin() + start_index;

        const auto insert_iter = [&buckets, start_iter] {
            const auto first =
                std::find_if(start_iter, buckets.end(), &valueless);

            if (first == buckets.end()) {
                const auto second = std::find_if(buckets.begin(), start_iter,
                                                 &valueless);

                return second == start_iter ? buckets.end() : second;
            }

            return first;
        }();

        if (insert_iter == buckets.end()) {
            return nullptr;
        }

        insert_iter->emplace(std::move(value));
        return insert_iter;
    }

    template <
        typename Key, typename Equal,
        std::enable_if_t<
            std::is_invocable_v<const Equal&, const Key&, const Value&>
            || std::is_invocable_v<const Equal&, const Value&, const Key&>,
            int
        > = 0
    >
    static BucketSpanIterT erase(const BucketSpanT buckets, const Key &key,
                                 const std::size_t hash, const Equal &equal) {
        const std::size_t start_index = hash % buckets.size();
        const auto start_iter = buckets.begin() + start_index;

        const auto delete_iter = [&buckets, start_iter, &key, &equal] {
            const auto is_equal = [&key, &equal](const Bucket<Value> &bucket) {
                if (!bucket.has_value()) {
                    return false;
                }

                if constexpr (
                    std::is_invocable_v<const Equal&, const Key&, const Value&>
                ) {
                    return std::invoke(equal, key, bucket.get_value().value());
                } else {
                    return std::invoke(equal, bucket.get_value().value(), key);
                }
            }();

            const auto first =
                std::find_if(start_iter, buckets.end(), is_equal);
        }();
    }

private:
    static bool valueless(const Bucket<Value> &bucket) noexcept {
        return !bucket.has_value();
    }

};
    
} // namespace gregjm::containers::set


#endif
