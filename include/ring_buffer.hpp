#ifndef GREGJM_CONTAINERS_RING_BUFFER_HPP
#define GREGJM_CONTAINERS_RING_BUFFER_HPP

#include "traits.hpp"

#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace gregjm {
namespace containers {

template <typename T, typename Allocator = std::allocator<T>>
class RingBuffer;

template <typename T>
class RingBufferIterator;

template <typename T>
class RingBufferConstIterator;

template <typename T, typename Allocator>
class RingBuffer {
    using TraitsT = std::allocator_traits<Allocator>;

public:
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using iterator = RingBufferIterator<T>;
    using const_iterator = RingBufferConstIterator<T>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using allocator_type = Allocator;
    using difference_type = std::ptrdiff_t;
    using size_type = std::size_t;
    using pointer = typename std::allocator_traits<allocator_type>::pointer;
    using const_pointer =
        typename std::allocator_traits<allocator_type>::const_pointer;

    RingBuffer() noexcept(noexcept(allocator_type{ }));

    explicit RingBuffer(const allocator_type &allocator) noexcept;

    RingBuffer(size_type count, const T &value,
               const allocator_type &allocator = allocator_type{ });

    explicit RingBuffer(size_type count,
                        const allocator_type &allocator = allocator_type{ });

    template <typename InputIterator,
              std::enable_if_t<IS_INPUT_ITERATOR<InputIterator>, int> = 0>
    RingBuffer(InputIterator first, InputIterator last,
               const allocator_type &allocator = allocator_type{ });

    RingBuffer(const RingBuffer &other);

    RingBuffer(const RingBuffer &other, const allocator_type &allocator);

    RingBuffer(RingBuffer &&other) noexcept;

    RingBuffer(RingBuffer &&other, const allocator_type &allocator);

    RingBuffer(std::initializer_list<value_type> list,
               const allocator_type &allocator = allocator_type{ });

    ~RingBuffer() {
        clear();

        if (data_) {
            TraitsT::deallocate(allocator_, data_);
            data_ = nullptr;
            capacity_ = 0;
        }
    }

    RingBuffer& operator=(const RingBuffer &other);

    RingBuffer& operator=(RingBuffer &&other)
    noexcept(std::allocator_traits<Allocator>
                 ::propagate_on_container_move_assignment_v
             || std::allocator_traits<Allocator>::is_always_equal_v);

    RingBuffer& operator=(std::initializer_list<value_type> list);

    void assign(size_type count, const value_type &value);

    template <typename InputIterator,
              std::enable_if_t<IS_INPUT_ITERATOR<InputIterator>, int> = 0>
    void assign(InputIterator first, InputIterator last);

    void assign(std::initializer_list<value_type> list);

    reference at(size_type index);

    const_reference at(size_type index) const;

    reference operator[](size_type index);

    const_reference operator[](size_type index) const;

    reference front();

    const_reference front() const;

    reference back();

    const_reference back() const;

    value_type* data() noexcept;

    const value_type* data() const noexcept;

    iterator begin() noexcept {
        return { data(), size(), front_index() };
    }

    const_iterator begin() const noexcept {
        return cbegin();
    }

    const_iterator cbegin() const noexcept {
        return { data(), size(), front_index() };
    }

    iterator end() noexcept {
        return { data(), size(), end_index() };
    }

    const_iterator end() const noexcept {
        return cend();
    }

    const_iterator cend() const noexcept {
        return { data(), size(), end_index() };
    }

    reverse_iterator rbegin() noexcept {
        return { end() };
    }

    const_reverse_iterator rbegin() const noexcept {
        return crbegin();
    }

    const_reverse_iterator crbegin() const noexcept {
        return { cend() };
    }

    reverse_iterator rend() noexcept {
        return { begin() };
    }

    const_reverse_iterator rend() const noexcept {
        return crend();
    }

    const_reverse_iterator crend() const noexcept {
        return { cbegin() };
    }

    bool empty() const noexcept {
        return size() == 0;
    }

    size_type size() const noexcept {
        return size_;
    }

    size_type max_size() const noexcept {
        return static_cast<size_type>(TraitsT::max_size(allocator_));
    }

    void reserve(size_type new_capacity) {
        if (new_capacity <= capacity_) {
            return;
        }

        pointer new_data = TraitsT::allocate(allocator_, new_capacity);

        if constexpr (std::is_nothrow_move_constructible_v<value_type>) {
            pointer new_element = new_data;

            for (auto &element : *this) {
                TraitsT::construct(allocator_, new_element,
                                   std::move(element));
                ++new_element;
            }
        } else { // copying may throw, but it won't mutate our old buffer
            pointer new_element = new_data;

            try {
                for (const auto &element : *this) {
                    TraitsT::construct(allocator_, new_element,
                                       element);
                    ++new_element;
                }
            } catch (...) { // only the constructor might throw here
                for (pointer element = 0; element < new_element; ++element) {
                    TraitsT::destroy(allocator_, element);
                }

                TraitsT::deallocate(allocator_, new_data, new_capacity);
                throw;
            }
        }

        clear(); // destruct all elements

        using std::swap;
        swap(data_, new_data); // allocator pointers must be nothrow

        TraitsT::deallocate(allocator_, new_data, new_capacity);

        capacity_ = new_capacity;
    }

    void shrink_to_fit() {
        if (capacity_ == size_) {
            return;
        }
    }

    void clear() noexcept {
        for (auto &element : *this) {
            TraitsT::destroy(allocator_, { &element });
        }

        size_ = 0;
        begin_ = 0;
    }

    iterator insert(const_iterator position, const value_type &value);

    iterator insert(const_iterator position, value_type &&value);

    iterator insert(const_iterator position, size_type count,
                    const value_type &value);

    template <typename InputIterator,
              std::enable_if_t<IS_INPUT_ITERATOR<InputIterator>, int> = 0>
    iterator insert(const_iterator position, InputIterator first,
                    InputIterator last);

    iterator insert(const_iterator position,
                    std::initializer_list<value_type> list);

    template <typename ...Args,
              std::enable_if_t<std::is_constructible_v<value_type, Args...>, int> = 0>
    iterator emplace(const_iterator position, Args &&...args);

    iterator erase(const_iterator position);

    iterator erase(const_iterator first, const_iterator last);

    void push_back(const value_type &value);

    void push_back(value_type &&value);

    template <typename ...Args,
              std::enable_if_t<std::is_constructible_v<value_type, Args...>, int> = 0>
    void emplace_back(Args &&...args);

    void push_front(const value_type &value);

    void push_front(value_type &&value);

    template <typename ...Args,
              std::enable_if_t<std::is_constructible_v<value_type, Args...>, int> = 0>
    void emplace_front(Args &&...args);

    void pop_back();

    void pop_front();

    void resize(size_type count);

    void resize(size_type count, const value_type &value);

    void swap(RingBuffer &other)
    noexcept(std::allocator_traits<allocator_type>
                 ::propagate_on_container_swap_v
             || std::allocator_traits<allocator_type>::is_always_equal_v);

private:
    size_type front_index() const noexcept {
        return begin_;
    }

    size_type back_index() const noexcept {
        return (begin_ + size_) % capacity_;
    }

    size_type end_index() const noexcept {
        return (begin_ + size_ + 1) % capacity_;
    }

    size_type rend_index() const noexcept {
        return (begin_ - 1) % capacity_;
    }

    pointer data_ = nullptr;
    size_type size_ = 0; // invariant: size <= capacity_
    size_type capacity_ = 0;
    size_type begin_ = 0; // invariant: begin_ < capacity_
    allocator_type allocator_{ };
};

} // namespace containers
} // namespace gregjm


#endif
