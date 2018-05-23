#ifndef GREGJM_CONTAINERS_SET_ROBIN_HOOD_HPP
#define GREGJM_CONTAINERS_SET_ROBIN_HOOD_HPP

#include "set/dib_bucket.hpp"
#include "utility.hpp"

#include <cstddef>

#include <gsl/gsl>

namespace gregjm::containers::set {

template <typename T>
class RobinHood;

template <typename T>
class RobinHoodIterator;

template <typename T>
class RobinHood {
public:
    using BucketT = DibBucket<T>;
    using SpanT = gsl::span<BucketT>;
    using IteratorT = RobinHoodIterator<T>;

    explicit RobinHood(const SpanT buckets) noexcept
    : buckets_{ buckets } { }

    const SpanT& buckets() const noexcept {
        return buckets_;
    }

    IteratorT begin() const noexcept;

    IteratorT cbegin() const noexcept;

    IteratorT end() const noexcept;

    IteratorT cend() const noexcept;

    std::size_t num_occupied() const noexcept;

    template <
        typename Key, typename Equal,
        std::enable_if_t<std::is_invocable_v<Equal, Key, const T&>
                         || std::is_invocable_v<Equal, const T&, Key>,
                         int> = 0
    >
    typename SpanT::const_iterator
    find(Key &&key, const std::size_t hash, Equal &&equal) const noexcept;

    typename SpanT::iterator
    insert(T &&value, const std::size_t hash);

    template <
        typename Key, typename Equal,
        std::enable_if_t<std::is_invocable_v<Equal, Key, const T&>
                        || std::is_invocable_v<Equal, const T&, Key>,
                        int> = 0
    >
    typename SpanT::iterator
    erase(Key &&key, const std::size_t hash, Equal &&equal);

    template <typename Hasher,
              std::enable_if_t<IS_HASHER_FOR<Hasher, T>, int> = 0>
    SpanT move_to(Hasher &&hasher, SpanT new_buckets);

private:
    SpanT buckets_;
    std::size_t num_occupied_;
};

} // namespace gregjm::containers::set

#endif
