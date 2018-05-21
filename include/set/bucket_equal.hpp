#ifndef GREGJM_CONTAINERS_SET_BUCKET_EQUAL_HPP
#define GREGJM_CONTAINERS_SET_BUCKET_EQUAL_HPP

#include <functional>
#include <type_traits>

namespace gregjm::containers::set {

template <
    typename Bucket,
    typename Equal,
    std::enable_if_t<
        std::is_invocable_v<const Equal&, const typename Bucket::ValueT&,
                            const typename Bucket::ValueT&>,
        int
    > = 0
>
struct BucketEqual {
    using ValueT = typename Bucket::ValueT;

    BucketEqual(const Equal &eq) : equals{ eq } { }

    bool operator()(const ValueT &lhs, const ValueT &rhs) const {
        return std::invoke(equals, lhs, rhs);
    }

    bool operator()(const Bucket &lhs, const Bucket &rhs) const {
        if (!lhs.has_value() || !rhs.has_value()) {
            return false;
        }

        const ValueT &lhs_value = lhs.unwrap();
        const ValueT &rhs_value = rhs.unwrap();

        return std::invoke(equals, lhs_value, rhs_value);
    }

    bool operator()(const ValueT &lhs, const Bucket &rhs) const {
        if (!rhs.has_value()) {
            return false;
        }

        const ValueT &value = rhs.unwrap();
        return std::invoke(equals, lhs, value);
    }

    bool operator()(const Bucket &lhs, const ValueT &rhs) const {
        if (!lhs.has_value()) {
            return false;
        }

        const ValueT &value = lhs.unwrap();
        return std::invoke(equals, value, rhs);
    }

    template <
        typename U,
        std::enable_if_t<
            std::is_invocable_v<const Equal&, const ValueT&, U>
            || std::is_invocable_v<const Equal&, U, const ValueT&>,
            int
        > = 0
    >
    bool operator()(const ValueT &lhs, U &&rhs) const {
        if constexpr (
            std::is_invocable_v<const Equal&, const ValueT&, U>
        ) {
            return std::invoke(equals, lhs, std::forward<U>(rhs));
        } else {
            return std::invoke(equals, std::forward<U>(rhs), lhs);
        }
    }

    template <
        typename U,
        std::enable_if_t<
            std::is_invocable_v<const Equal&, const ValueT&, U>
            || std::is_invocable_v<const Equal&, U, const ValueT&>,
            int
        > = 0
    >
    bool operator()(U &&lhs, const ValueT &rhs) const {
        if constexpr (
            std::is_invocable_v<const Equal&, const ValueT&, U>
        ) {
            return std::invoke(equals, rhs, std::forward<U>(lhs));
        } else {
            return std::invoke(equals, std::forward<U>(lhs), rhs);
        }
    }

    template <
        typename U,
        std::enable_if_t<
            std::is_invocable_v<const Equal&, const ValueT&, U>
            || std::is_invocable_v<const Equal&, U, const ValueT&>,
            int
        > = 0
    >
    bool operator()(const Bucket &lhs, U &&rhs) const {
        if (!lhs.has_value()) {
            return false;
        }

        const ValueT &value = lhs.unwrap();

        if constexpr (
            std::is_invocable_v<const Equal&, const ValueT&, U>
        ) {
            return std::invoke(equals, value, std::forward<U>(rhs));
        } else {
            return std::invoke(equals, std::forward<U>(rhs), value);
        }
    }

    template <
        typename U,
        std::enable_if_t<
            std::is_invocable_v<const Equal&, const ValueT&, U>
            || std::is_invocable_v<const Equal&, U, const ValueT&>,
            int
        > = 0
    >
    bool operator()(U &&lhs, const Bucket &rhs) const {
        if (!rhs.has_value()) {
            return false;
        }

        const ValueT &value = rhs.unwrap();

        if constexpr (
            std::is_invocable_v<const Equal&, const ValueT&, U>
        ) {
            return std::invoke(equals, value, std::forward<U>(lhs));
        } else {
            return std::invoke(equals, std::forward<U>(lhs), value);
        }
    }

    Equal equals;
};

} // namespace gregjm::containers::set

#endif
