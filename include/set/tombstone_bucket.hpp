#ifndef GREGJM_CONTAINERS_SET_TOMBSTONE_BUCKET_HPP
#define GREGJM_CONTAINERS_SET_TOMBSTONE_BUCKET_HPP

#include "utility.hpp"

#include <functional>
#include <optional>
#include <utility>
#include <variant>

namespace gregjm::containers::set {

template <typename T>
class TombstoneBucket {
private:
    class EmptyBucket { };

    class DeletedBucket { }; // tombstone
    
    using UnderlyingT = std::variant<EmptyBucket, DeletedBucket, T>;

public:
    using ValueT = T;

    TombstoneBucket() noexcept = default;

    template <typename ...As,
              std::enable_if_t<std::is_constructible_v<T, As...>, int> = 0>
    TombstoneBucket(As &&...args)
    noexcept(std::is_nothrow_constructible_v<T, As...>)
    : data_{ std::in_place_type<T>, std::forward<As>(args)... } { }

    bool is_empty() const noexcept {
        return std::holds_alternative<EmptyBucket>(data_);
    }

    bool is_deleted() const noexcept {
        return std::holds_alternative<DeletedBucket>(data_);
    }

    bool has_value() const noexcept {
        return std::holds_alternative<T>(data_);
    }

    T& unwrap() {
        return std::get<T>(data_);
    }

    const T& unwrap() const {
        return std::get<T>(data_);
    }

    template <typename ...As,
              std::enable_if_t<std::is_constructible_v<T, As...>, int> = 0>
    void emplace(As &&...args)
    noexcept(std::is_nothrow_constructible_v<T, As...>) {
        if constexpr (std::is_nothrow_constructible_v<T, As...>) {
            data_.template emplace<T>(std::forward<As>(args)...);
        } else {
            try {
                data_.template emplace<T>(std::forward<As>(args)...);
            } catch (...) {
                data_.template emplace<EmptyBucket>();
                
                throw;
            }
        }
    }

    void set_empty() noexcept {
        if (is_empty()) {
            return;
        }

        data_.template emplace<EmptyBucket>();
    }

    void set_deleted() noexcept {
        if (is_deleted()) {
            return;
        }

        data_.template emplace<DeletedBucket>();
    }

    void swap(TombstoneBucket &other)
    noexcept(std::is_nothrow_swappable_v<UnderlyingT>) {
        adl_swap(data_, other.data_);
    }

private:
    UnderlyingT data_ = EmptyBucket{ }; 
};

template <typename T>
void swap(TombstoneBucket<T> &lhs, TombstoneBucket<T> &rhs)
noexcept(noexcept(lhs.swap(rhs))) {
    lhs.swap(rhs);
}

template <typename K, typename P>
struct TombstoneEmptyOrPred {
    constexpr TombstoneEmptyOrPred(K &&key, P &&pred) noexcept
    : key_{ std::forward<K>(key) }, pred_{ std::forward<P>(pred) } { }

    constexpr TombstoneEmptyOrPred(const TombstoneEmptyOrPred &other) noexcept
    : key_{ std::forward<K>(other.key_) },
      pred_{ std::forward<P>(other.pred_) } { }

    constexpr TombstoneEmptyOrPred(TombstoneEmptyOrPred &&other) noexcept
    : key_{ std::forward<K>(other.key_) },
      pred_{ std::forward<P>(other.pred_) } { }

    template <
        typename T,
        std::enable_if_t<
            IS_BINARY_PREDICATE<P, const T&, K>
            || IS_BINARY_PREDICATE<P, K, const T&>,
            int
        > = 0
    >
    bool operator()(const TombstoneBucket<T> &bucket) {
        if (bucket.is_empty()) {
            return true;
        } else if (!bucket.has_value()) {
            return false;
        }

        if constexpr (IS_BINARY_PREDICATE<P, const T&, K>) {
            return std::invoke(std::forward<P>(pred_), bucket.unwrap(),
                               std::forward<K>(key_));
        } else {
            return std::invoke(std::forward<P>(pred_), std::forward<K>(key_),
                               bucket.unwrap());
        }
    }

private:
    K &&key_;
    P &&pred_;
};

} // namespace gregjm::containers::set

#endif
