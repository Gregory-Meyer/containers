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
    class EmptyBucket { };

    class DeletedBucket { }; // tombstone

    using UnderlyingT = std::variant<EmptyBucket, DeletedBucket, T>;

    UnderlyingT data_ = EmptyBucket{ }; 
};

template <typename T>
void swap(TombstoneBucket<T> &lhs, TombstoneBucket<T> &rhs)
noexcept(noexcept(lhs.swap(rhs))) {
    lhs.swap(rhs);
}

} // namespace gregjm::containers::set

#endif
