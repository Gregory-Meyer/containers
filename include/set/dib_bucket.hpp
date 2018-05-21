#ifndef GREGJM_CONTAINERS_SET_DIB_BUCKET_HPP
#define GREGJM_CONTAINERS_SET_DIB_BUCKET_HPP

#include <optional>
#include <tuple>
#include <utility>

namespace gregjm::containers::set {

template <typename T>
class DibBucket {
public:
    using ValueT = T;

    DibBucket() noexcept = default;

    template <typename ...As,
              std::enable_if_t<std::is_constructible_v<T, As...>, int> = 0>
    DibBucket(As &&...args)
    noexcept(std::is_nothrow_constructible_v<T, As...>)
    : data_{ std::in_place, std::piecewise_construct,
             std::forward_as_tuple(std::forward<As>(args)...),
             std::forward_as_tuple(0) } { }

    bool is_empty() const noexcept {
        return !data_.has_value();
    }

    bool has_value() const noexcept {
        return data_.has_value();
    }

    bool has_distance() const noexcept {
        return has_value();
    }

    T& unwrap() {
        return data_.value().first;
    }

    const T& unwrap() const {
        return data_.value().first;
    }

    std::size_t& unwrap_distance() {
        return data_.value().second;
    }

    std::size_t unwrap_distance() const {
        return data_.value().second;
    }

    template <typename ...As,
              std::enable_if_t<std::is_constructible_v<T, As...>, int> = 0>
    void emplace(As &&...args)
    noexcept(std::is_nothrow_constructible_v<T, As...>) {
        data_.emplace(std::piecewise_construct,
                      std::forward_as_tuple(std::forward<As>(args)...),
                      std::forward_as_tuple(0));
    }

    void set_empty() noexcept {
        data_.reset();
    }

private:
    std::optional<std::pair<T, std::size_t>> data_ = std::nullopt;
};

} // namespace gregjm::containers::set

#endif
