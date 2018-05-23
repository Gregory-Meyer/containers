#ifndef GREGJM_CONTAINERS_SET_HPP
#define GREGJM_CONTAINERS_SET_HPP

#include "set/bucket_equal.hpp"
#include "set/linear_probe.hpp"
#include "utility.hpp"

#include <algorithm>
#include <functional>
#include <iterator>
#include <memory>

namespace gregjm::containers {

template <typename Key, typename Hash = std::hash<Key>,
          typename KeyEqual = std::equal_to<>,
          typename Allocator = std::allocator<Key>,
          typename HashPolicy = set::LinearProbe<Key>>
class Set {
public:
    using value_type = Key;
    using reference = Key&;
    using const_reference = const Key&;
    using iterator = typename HashPolicy::iterator;
    using const_iterator = iterator;
    using size_type = std::size_t;
    using difference_type = IteratorDifferenceType<iterator>;
    using allocator_type = Allocator;
    using pointer = typename std::allocator_traits<Allocator>::pointer;
    using const_pointer =
        typename std::allocator_traits<Allocator>::const_pointer;

    using key_type = Key;
    using hasher = Hash;
    using key_equal = KeyEqual;

    Set() = default;

    template <
        typename I,
        std::enable_if_t<
            IS_INPUT_ITERATOR<I>
            && std::is_constructible_v<Key, IteratorReferenceT<I>
        >, int> = 0
    >
    Set(const I first, const I last) {
        insert(first, last);
    }

    explicit Set(const std::initializer_list<Key> list)
    : Set(list.begin(), list.end()) { }

    allocator_type get_allocator() const {
        return buckets_.get_allocator();
    }

    iterator begin() const noexcept {
        return policy_.begin();
    }

    const_iterator cbegin() const noexcept {
        return begin();
    }

    iterator end() const noexcept {
        return policy_.end();
    }

    const_iterator cend() const noexcept {
        return end();
    }

    [[nodiscard]] bool empty() const noexcept {
        return size() == 0;
    }

    [[nodiscard]] size_type size() const noexcept {
        return static_cast<size_type>(policy_.num_occupied());
    }

    [[nodiscard]] size_type max_size() const noexcept {
        return static_cast<size_type>(buckets_.max_size());
    }

    [[nodiscard]] size_type capacity() const noexcept {
        return static_cast<size_type>(buckets_.size() / 2);
    }

    void clear() noexcept {
        policy_.clear();
    }

    std::pair<iterator, bool> insert(const Key &key) {
        return emplace(key);
    }

    std::pair<iterator, bool> insert(Key &&key) {
        return emplace(key);
    }

    template <
        typename I,
        std::enable_if_t<
            IS_INPUT_ITERATOR<I>
            && std::is_constructible_v<Key, IteratorReferenceT<I>
        >, int> = 0
    >
    std::size_t insert(I first, const I last) {
        if constexpr (const Range r(first, last); r.has_size()) {
            reserve(next_power_of_2(size() + r.size()));
        }

        std::size_t num_inserted = 0;
        for (auto &&element : Range(first, last)) {
            if (insert(element).second) {
                ++num_inserted;
            }
        }

        return num_inserted;
    }

    std::size_t insert(const std::initializer_list<Key> list) {
        return insert(list.begin(), list.end());
    }

    template <typename ...Args,
              std::enable_if_t<std::is_constructible_v<Key, Args...>, int> = 0>
    std::pair<iterator, bool> emplace(Args &&...args) {
        Key key(std::forward<Args>(args)...);

        const std::size_t hash = hash_key(key);

        if (const auto found_iter = policy_.find(key, hash, equal_);
            found_iter != policy_.cend()) {
            return { found_iter, false };
        }

        if (should_realloc()) {
            realloc_and_move(next_capacity());
        }

        if (const auto inserted_iter = policy_.insert(std::move(key), hash);
            inserted_iter != policy_.end()) {
            return { inserted_iter, true };
        }

        throw std::logic_error{ "Set::emplace" };
    }

    iterator erase(const const_iterator iter) {
        return policy_.erase(*iter);
    }

    iterator erase(const const_iterator first, const const_iterator last) {
        iterator last_erased = first;

        for (const auto &key : make_range(first, last)) {
            last_erased = erase(key);
        }

        return last_erased;
    }

    iterator erase(const Key &key) {
        const std::size_t hash = hash_key(key);

        return policy_.erase(key, hash, equal_);
    }

    template <typename U, typename H,
              std::enable_if_t<IS_HASHER_FOR<H, U>, int> = 0>
    iterator erase(U &&key, H &&hasher) {
        const std::size_t hash =
            std::invoke(std::forward<H>(hasher), std::forward<U>(key));

        return policy_.erase(std::forward<U>(key), hash, equal_);
    }

    void swap(Set &other) {
        adl_swap(buckets_, other.buckets_);
        adl_swap(hasher_, other.hasher_);
        adl_swap(equal_, other.equal_);
        adl_swap(policy_, other.policy_);
    }

    [[nodiscard]] size_type count(const Key &key) const noexcept {
        const std::size_t hash = hash_key(key);

        const auto iter = policy_.find(key, hash, equal_);

        return iter != policy_.cend() ? 1 : 0;
    }

    template <typename U, typename H,
              std::enable_if_t<IS_HASHER_FOR<H, U>, int> = 0>
    [[nodiscard]] size_type count(U &&key, H &&hasher) const noexcept {
        const std::size_t hash =
            std::invoke(std::forward<H>(hasher), std::forward<U>(key));

        const auto iter = policy_.find(std::forward<U>(key), hash, equal_);

        return iter != policy_.cend() ? 1 : 0;
    }

    [[nodiscard]] double load_factor() const noexcept {
        return static_cast<double>(size()) / static_cast<double>(capacity());
    }

    void reserve(const size_type new_capacity) {
        if (new_capacity < capacity()) {
            return;
        }

        realloc_and_move(next_power_of_2(new_capacity * 2));
    }

    hasher hash_function() const {
        return hasher_;
    }

    key_equal key_eq() const {
        return equal_.equals;
    }

    iterator find(const Key &key) noexcept {
        return std::as_const(*this).find(key);
    }

    const_iterator find(const Key &key) const noexcept {
        const std::size_t hash = hash_key(key);

        return policy_.find(key, hash, equal_);
    }

    template <typename U, typename H,
              std::enable_if_t<IS_HASHER_FOR<H, U>, int> = 0>
    const_iterator find(U &&key, H &&hasher) const noexcept {
        const std::size_t hash = std::invoke(std::forward<H>(hasher),
                                             std::forward<U>(key));

        return policy_.find(std::forward<U>(key), hash, equal_);
    }

    friend bool operator==(const Set &lhs, const Set &rhs) noexcept {
        for (const auto &key : lhs) {
            if (rhs.find(key) == rhs.cend()) {
                return false;
            }
        }

        return true;
    }

    friend bool operator!=(const Set &lhs, const Set &rhs) noexcept {
        return !(lhs == rhs);
    }

private:
    using KeyAllocT = Allocator;
    using KeyTraitsT = std::allocator_traits<KeyAllocT>;

    using BucketT = typename HashPolicy::bucket_type;
    using AllocT =
        typename KeyTraitsT::template rebind_alloc<BucketT>;
    using BucketEqualT = set::BucketEqual<BucketT, KeyEqual>;
    using BucketViewT = typename HashPolicy::view;

    // realloc if empty or load factor > 1/2
    bool should_realloc() {
        return buckets_.empty() || size() + 1 > capacity();
    }

    void realloc_and_move(const size_type new_capacity) {
        std::vector<BucketT, AllocT> new_buckets(new_capacity * 2);

        adl_swap(buckets_, new_buckets);

        policy_.move_to(hasher_, view());
    }

    std::size_t hash_key(const Key &key) const noexcept {
        return std::invoke(hasher_, key);
    }

    BucketViewT view() noexcept {
        const auto data =
            static_cast<typename BucketViewT::pointer>(buckets_.data());
        const auto size =
            static_cast<typename BucketViewT::size_type>(buckets_.size());

        return { data, size };
    }

    size_type next_capacity() const noexcept {
        static constexpr size_type MIN_CAPACITY = 2;

        if (capacity() == 0) {
            return MIN_CAPACITY;
        }

        return next_power_of_2(capacity() * 2);
    }

    std::vector<BucketT, AllocT> buckets_;

    Hash hasher_{ };
    BucketEqualT equal_{ KeyEqual{ } };
    HashPolicy policy_{ view() };
};

template <typename K, typename H, typename E, typename A, typename P>
void swap(Set<K, H, E, A, P> &lhs, Set<K, H, E, A, P> &rhs) {
    lhs.swap(rhs);
}

} // namespace gregjm::containers

#endif
