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
    using iterator = typename HashPolicy::IteratorT;
    using const_iterator = iterator;
    using size_type = std::size_t;
    using difference_type = IteratorDifferenceType<iterator>;

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

    [[nodiscard]] bool empty() const noexcept {
        return size() == 0;
    }

    [[nodiscard]] size_type size() const noexcept {
        return static_cast<size_type>(policy_.num_occupied());
    }

    [[nodiscard]] size_type capacity() const noexcept {
        return static_cast<size_type>(buckets_.size());
    }

    bool insert(const Key &key) {
        return emplace(key);
    }

    bool insert(Key &&key) {
        return emplace(key);
    }

    const_iterator begin() const noexcept {
        return policy_.begin();
    }

    const_iterator cbegin() const noexcept {
        return begin();
    }

    const_iterator end() const noexcept {
        return policy_.end();
    }

    const_iterator cend() const noexcept {
        return end();
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
            if (insert(element)) {
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
    bool emplace(Args &&...args) {
        Key key(std::forward<Args>(args)...);

        const std::size_t hash = hash_key(key);

        if (policy_.find(key, hash, equal_) != policy_.buckets().cend()) {
            return false;
        }

        if (should_realloc()) {
            realloc_and_move(next_capacity());
        }

        if (policy_.insert(std::move(key), hash) != policy_.buckets().end()) {
            return true;
        }

        return false;
    }

    bool erase(const Key &key) {
        const std::size_t hash = hash_key(key);

        if (policy_.erase(key, hash, equal_) != policy_.buckets().end()) {
            return true;
        }

        return false;
    }

    [[nodiscard]] bool contains(const Key &key) const noexcept {
        const std::size_t hash = hash_key(key);

        const auto iter = policy_.find(key, hash, equal_);

        return iter != policy_.buckets().cend();
    }

    template <typename U, typename UHasher>
    [[nodiscard]] bool contains(U &&key, UHasher &&hasher) const noexcept {
        const std::size_t hash =
            std::invoke(std::forward<UHasher>(hasher), std::forward<U>(key));

        const auto iter = policy_.find(std::forward<U>(key), hash, equal_);

        return iter != policy_.buckets().cend();
    }

    void reserve(const size_type new_capacity) {
        if (new_capacity * 2 < capacity()) {
            return;
        }

        realloc_and_move(next_power_of_2(new_capacity * 2));
    }

    [[nodiscard]] double load_factor() const noexcept {
        return static_cast<double>(size()) / static_cast<double>(capacity());
    }

    void swap(Set &other) {
        adl_swap(buckets_, other.buckets_);
        adl_swap(hasher_, other.hasher_);
        adl_swap(equal_, other.equal_);
        adl_swap(policy_, other.policy_);
    }

private:
    using KeyAllocT = Allocator;
    using KeyTraitsT = std::allocator_traits<KeyAllocT>;

    using BucketT = typename HashPolicy::BucketT;
    using AllocT = typename KeyTraitsT::template rebind_alloc<BucketT>;
    using BucketEqualT = set::BucketEqual<BucketT, KeyEqual>;
    using BucketSpanT = typename HashPolicy::SpanT;

    // realloc if empty or load factor > 1/2
    bool should_realloc() {
        return buckets_.empty() || capacity() / (size() + 1) < 2;
    }

    void realloc_and_move(size_type new_capacity) {
        std::vector<BucketT, AllocT> new_buckets(new_capacity);

        {
            using std::swap;
            swap(buckets_, new_buckets);
        }

        policy_.move_to(hasher_, span());
    }

    std::size_t hash_key(const Key &key) const noexcept {
        return std::invoke(hasher_, key);
    }

    BucketSpanT span() noexcept {
        const auto span_size =
            static_cast<typename BucketSpanT::size_type>(buckets_.size());

        return { buckets_.data(), span_size };
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
    HashPolicy policy_{ span() };
};

template <typename K, typename H, typename E, typename A, typename P>
void swap(Set<K, H, E, A, P> &lhs, Set<K, H, E, A, P> &rhs) {
    lhs.swap(rhs);
}

} // namespace gregjm::containers

#endif
