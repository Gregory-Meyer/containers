#ifndef GREGJM_CONTAINERS_RING_BUFFER_HPP
#define GREGJM_CONTAINERS_RING_BUFFER_HPP

#include "traits.hpp"

#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <type_traits>
#include <vector>

namespace gregjm {
namespace containers {

class RingBufferIterator;

class RingBufferConstIterator;

template <typename T, typename Allocator = std::allocator<T>>
class RingBuffer {
public:
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using iterator = RingBufferIterator;
    using const_iterator = RingBufferConstIterator;
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

    ~RingBuffer();

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

    iterator begin() noexcept;

    const_iterator begin() const noexcept;

    const_iterator cbegin() const noexcept;

    iterator end() noexcept;

    const_iterator end() const noexcept;

    const_iterator cend() const noexcept;

    reverse_iterator rbegin() noexcept;

    const_reverse_iterator rbegin() const noexcept;

    const_reverse_iterator crbegin() const noexcept;

    reverse_iterator rend() noexcept;

    const_reverse_iterator rend() const noexcept;

    const_reverse_iterator crend() const noexcept;

    bool empty() const noexcept;

    size_type size() const noexcept;

    size_type max_size() const noexcept;

    void reserve(size_type new_capacity);

    void shrink_to_fit();

    void clear() noexcept;

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
    pointer data_ = nullptr;
    size_type size_ = 0;
    size_type capacity_ = 0;
    size_type begin_ = 0;
    allocator_type allocator_{ };
};

} // namespace containers
} // namespace gregjm


#endif
