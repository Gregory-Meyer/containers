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
class RobinHoodIterator {
public:
    friend RobinHood<T>;
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

    explicit RobinHood(const view buckets) noexcept;

    iterator begin() const noexcept;

    const_iterator cbegin() const noexcept;

    iterator end() const noexcept;

    const_iterator cend() const noexcept;

    size_type num_occupied() const noexcept;

    void clear() noexcept;

    template <typename K, typename E,
              std::enable_if_t<IS_BINARY_PREDICATE<E, const T&, K>
                              || IS_BINARY_PREDICATE<E, K, const T&>,
                              int> = 0>
    const_iterator find(K &&key, const std::size_t hash,
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
    view buckets_;
    size_type num_occupied_;
};

template <typename T>
void swap(RobinHood<T> &lhs, RobinHood<T> &rhs) noexcept;

} // namespace gregjm::containers::set

#endif
