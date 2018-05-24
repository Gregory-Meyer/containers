#ifndef GREGJM_CONTAINERS_ARRAY_VIEW_HPP
#define GREGJM_CONTAINERS_ARRAY_VIEW_HPP

#include <cstddef>
#include <iterator>

template <typename T>
class ArrayView {
public:
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using iterator = T*;
    using const_iterator = const T*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using difference_type = std::ptrdiff_t;
    using size_type = std::size_t;
    using pointer = T*;
    using const_pointer = const T*;

    constexpr ArrayView(const pointer data, const size_type size) noexcept
    : data_{ data }, size_{ size } { }

    constexpr ArrayView(const pointer first, const pointer last) noexcept
    : data_{ first }, size_{ last - first } { }

    constexpr reference at(const size_type index) const {
        range_check(index);

        return (*this)[index];
    }

    constexpr reference operator[](const size_type index) const {
        return data()[index];
    }

    constexpr reference front() const {
        return data()[0];
    }

    constexpr reference back() const {
        return data()[size() - 1];
    }

    constexpr pointer data() const noexcept {
        return data_;
    }

    constexpr iterator begin() const noexcept {
        return data();
    }

    constexpr const_iterator cbegin() const noexcept {
        return begin();
    }

    constexpr iterator end() const noexcept {
        return data() + size();
    }

    constexpr const_iterator cend() const noexcept {
        return end();
    }

    constexpr reverse_iterator rbegin() const noexcept {
        return { end() };
    }

    constexpr const_reverse_iterator crbegin() const noexcept {
        return { cend() };
    }

    constexpr reverse_iterator rend() const noexcept {
        return { begin() };
    }

    constexpr const_reverse_iterator crend() const noexcept {
        return { cbegin() };
    }

    constexpr bool empty() const noexcept {
        return size_ == 0;
    }

    constexpr size_type size() const noexcept {
        return size_;
    }

private:
    constexpr void range_check(const size_type index) const {
        if (index >= size_) {
            throw std::out_of_range{ "ArrayView::range_check" };
        }
    }

    pointer data_ = nullptr;
    size_type size_ = 0;
};

template <typename T>
class ConstArrayView {
public:
    using value_type = T;
    using reference = const T&;
    using const_reference = const T&;
    using iterator = const T*;
    using const_iterator = const T*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using difference_type = std::ptrdiff_t;
    using size_type = std::size_t;
    using pointer = const T*;
    using const_pointer = const T*;

    constexpr ConstArrayView(const pointer data, const size_type size) noexcept
    : data_{ data }, size_{ size } { }

    constexpr ConstArrayView(const pointer first, const pointer last) noexcept
    : data_{ first }, size_{ last - first } { }

    constexpr explicit ConstArrayView(const ArrayView<T> other) noexcept
    : data_{ other.data() }, size_{ other.size() } { }

    constexpr reference at(const size_type index) const {
        range_check(index);

        return (*this)[index];
    }

    constexpr reference operator[](const size_type index) const {
        return data()[index];
    }

    constexpr reference front() const {
        return data()[0];
    }

    constexpr reference back() const {
        return data()[size() - 1];
    }

    constexpr pointer data() const noexcept {
        return data_;
    }

    constexpr iterator begin() const noexcept {
        return data();
    }

    constexpr const_iterator cbegin() const noexcept {
        return begin();
    }

    constexpr iterator end() const noexcept {
        return data() + size();
    }

    constexpr const_iterator cend() const noexcept {
        return end();
    }

    constexpr reverse_iterator rbegin() const noexcept {
        return { end() };
    }

    constexpr const_reverse_iterator crbegin() const noexcept {
        return { cend() };
    }

    constexpr reverse_iterator rend() const noexcept {
        return { begin() };
    }

    constexpr const_reverse_iterator crend() const noexcept {
        return { cbegin() };
    }

    constexpr bool empty() const noexcept {
        return size_ == 0;
    }

    constexpr size_type size() const noexcept {
        return size_;
    }

private:
    constexpr void range_check(const size_type index) const {
        if (index >= size_) {
            throw std::out_of_range{ "ConstArrayView::range_check" };
        }
    }

    pointer data_ = nullptr;
    size_type size_ = 0;
};

#endif
