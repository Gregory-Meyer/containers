#ifndef GREGJM_CONTAINERS_SET_BUCKET_HPP
#define GREGJM_CONTAINERS_SET_BUCKET_HPP

#include <functional>
#include <optional>
#include <utility>
#include <variant>

namespace gregjm::containers::set {

template <typename Value>
class Bucket {
public:
    using RefT = std::reference_wrapper<Value>;
    using ConstRefT = std::reference_wrapper<Value>;

    bool is_empty() const noexcept {
        return std::holds_alternative<EmptyBucket>(data_);
    }

    bool is_deleted() const noexcept {
        return std::holds_alternative<DeletedBucket>(data_);
    }

    bool has_value() const noexcept {
        return std::holds_alternative<Value>(data_);
    }

    std::optional<RefT> get_value() noexcept {
        if (!has_value()) {
            return std::nullopt;
        }

        return { std::ref(std::get<Value>(data_)) };
    }

    std::optional<ConstRefT> get_value() const noexcept {
        if (!has_value()) {
            return std::nullopt;
        }

        return { std::cref(std::get<Value>(data_)) };
    }

    template <typename ...Args,
              std::enable_if_t<
                  std::is_constructible_v<Value, Args...>, int
              > = 0>
    void emplace(Args &&...args)
    noexcept(std::is_nothrow_constructible_v<Value, Args...>) {
        try {
            data_.template emplace<Value>(std::forward<Args>(args)...);
        } catch (...) {
            data_.template emplace<EmptyBucket>();
            
            throw;
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

private:
    class EmptyBucket { };

    class DeletedBucket { }; // tombstone

    std::variant<EmptyBucket, DeletedBucket, Value> data_; 
};

} // namespace gregjm::containers::set

#endif
