#ifndef GREGJM_CONTAINERS_RING_BUFFER_HPP
#define GREGJM_CONTAINERS_RING_BUFFER_HPP

#include "iterator_range.hpp"
#include "traits.hpp"

#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace gregjm {
namespace containers {

template <typename T, typename Allocator = std::allocator<T>>
class RingBuffer;

template <typename T>
class RingBufferIterator {
public:
    template <typename U, typename Allocator,
              std::enable_if_t<std::is_same_v<T, U>, int> = 0>
    friend class RingBuffer;
};

template <typename T>
class RingBufferConstIterator {
public:
    template <typename U, typename Allocator,
              std::enable_if_t<std::is_same_v<T, U>, int> = 0>
    friend class RingBuffer;

private:
};

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

    template <
        std::enable_if_t<
            std::is_default_constructible_v<allocator_type>, int
        > = 0
    >
    RingBuffer()
    noexcept(std::is_nothrow_default_constructible_v<allocator_type>) { }

    explicit RingBuffer(const allocator_type &allocator) noexcept
    : allocator_{ allocator } { }

    template <
        std::enable_if_t<std::is_copy_constructible_v<value_type>, int> = 0
    >
    RingBuffer(const size_type count, const value_type &value,
               const allocator_type &allocator = allocator_type{ })
    : allocator_{ allocator } {
        assign(count, value);
    }

    template <
        std::enable_if_t<std::is_default_constructible_v<value_type>
                         && std::is_copy_constructible_v<value_type>, int> = 0
    >
    explicit RingBuffer(const size_type count,
                        const allocator_type &allocator = allocator_type{ })
    : allocator_{ allocator } {
        assign(count, value_type{ });
    }

    template <typename InputIterator,
              std::enable_if_t<IS_INPUT_ITERATOR<InputIterator>, int> = 0>
    RingBuffer(const InputIterator first, const InputIterator last,
               const allocator_type &allocator = allocator_type{ })
    : allocator_{ allocator } {
        assign(first, last);
    }

    RingBuffer(const RingBuffer &other)
    : RingBuffer(other.cbegin(), other.cend(),
                 TraitsT::select_on_container_copy_construction(
                     other.get_allocator()
                 )) { }

    RingBuffer(const RingBuffer &other, const allocator_type &allocator)
    : RingBuffer(other.cbegin(), other.cend(), allocator) { }

    template <
        std::enable_if_t<
            std::is_default_constructible_v<allocator_type>, int
        > = 0
    >
    RingBuffer(RingBuffer &&other) noexcept {
        using std::swap;

        swap(data_, other.data_);
        swap(size_, other.size_);
        swap(capacity_, other.capacity_);
        swap(begin_, other.begin_);

        if constexpr (!TraitsT::is_always_equal::value) {
            swap(allocator_, other.allocator_);
        }
    }

    // TODO: implement
    RingBuffer(RingBuffer &&other, const allocator_type &allocator);

    RingBuffer(const std::initializer_list<value_type> list,
               const allocator_type &allocator = allocator_type{ })
    : RingBuffer(list.begin(), list.end(), allocator) { }

    ~RingBuffer() {
        clear();

        if (data_) {
            deallocate(data_);
            data_ = nullptr;
            capacity_ = 0;
        }
    }

    RingBuffer& operator=(const RingBuffer &other) {
        if (&other == this) {
            return *this;
        }

        RingBuffer copied = other;

        using std::swap;
        swap(*this, copied);

        return *this;
    }

    RingBuffer& operator=(RingBuffer &&other)
    noexcept(std::allocator_traits<Allocator>
                 ::propagate_on_container_move_assignment::value
             || std::allocator_traits<Allocator>::is_always_equal::value) {
        if (&other == this) {
            return *this;
        }

        RingBuffer moved = std::move(other);

        using std::swap;
        swap(*this, moved);

        return *this;
    }

    RingBuffer& operator=(const std::initializer_list<value_type> list) {
        RingBuffer initialized = list;

        using std::swap;
        swap(*this, initialized);

        return *this;
    }

    void assign(const size_type count, const value_type &value) {
        pointer new_data = allocate(count);
        pointer new_element_ptr = new_data;

        try {
            for (; new_element_ptr != new_data + count;
                 ++new_element_ptr) {
                construct(new_element_ptr, value);
            }
        } catch (...) {
            for (pointer element_ptr; element_ptr != new_element_ptr;
                 ++element_ptr) {
                destroy(element_ptr);
            }

            deallocate(new_data);

            throw;
        }

        clear();

        {
            using std::swap;
            swap(data_, new_data);
        }

        deallocate(new_data);

        capacity_ = count;
        size_ = count;
        begin_ = 0;
    }

    template <typename InputIterator,
              std::enable_if_t<IS_INPUT_ITERATOR<InputIterator>, int> = 0>
    void assign(const InputIterator first, const InputIterator last) {
        if constexpr (IS_FORWARD_ITERATOR<InputIterator>) {
            const auto new_size =
                static_cast<size_type>(std::distance(first, last));

            pointer new_data = allocate(new_size);
            pointer new_element_ptr = new_data;

            try {
                for (auto &&element : IteratorRange{ first, last }) {
                    construct(new_element_ptr++,
                              std::forward<decltype(element)>(element));
                }
            } catch (...) {
                for (pointer element_ptr = new_data;
                     element_ptr != new_element_ptr; ++element_ptr) {
                    destroy(element_ptr);
                }

                deallocate(new_data);

                throw;
            }

            clear();

            {
                using std::swap;
                swap(data_, new_data);
            }

            deallocate(new_data);

            size_ = new_size;
            capacity_ = new_size;
            begin_ = 0;
        } else {
            using BufferT = std::vector<value_type, allocator_type>;

            BufferT buffer(first, last, get_allocator());

            if constexpr (
                !std::is_nothrow_move_constructible_v<value_type>
                && std::is_nothrow_copy_constructible_v<value_type>
            ) {
                assign(buffer.cbegin(), buffer.cend());
            } else {
                const std::move_iterator move_begin{ buffer.begin() };
                const std::move_iterator move_end{ buffer.end() };

                assign(move_begin, move_end);
            }
        }
    }

    void assign(const std::initializer_list<value_type> list) {
        assign(list.begin(), list.end());
    }

    allocator_type get_allocator() const noexcept {
        return allocator_;
    }

    reference at(const size_type index) {
        range_check(index);

        return (*this)[index];
    }

    const_reference at(const size_type index) const {
        range_check(index);

        return (*this)[index];
    }

    reference operator[](const size_type index) {
        return (*this)[wrap_index(index)];
    }

    const_reference operator[](const size_type index) const {
        return (*this)[wrap_index(index)];
    }

    reference front() {
        return (*this)[front_index()];
    }

    const_reference front() const {
        return (*this)[front_index()];
    }

    reference back() {
        return (*this)[back_index()];
    }

    const_reference back() const {
        return (*this)[back_index()];
    }

    value_type* data() noexcept {
        return static_cast<value_type*>(data_);
    }

    const value_type* data() const noexcept {
        return static_cast<const value_type*>(data_);
    }

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

    void reserve(const size_type new_capacity) {
        if (new_capacity <= capacity()) {
            return;
        }

        pointer new_data = TraitsT::allocate(allocator_, new_capacity);

        if constexpr (std::is_nothrow_move_constructible_v<value_type>) {
            pointer new_element_ptr = new_data;

            for (auto &element : *this) {
                construct(new_element_ptr++, std::move(element));
            }
        } else { // copying may throw, but it won't mutate our old buffer
            pointer new_element_ptr = new_data;

            try {
                for (const auto &element : *this) {
                    construct(new_element_ptr++, element);
                }
            } catch (...) { // only the constructor might throw here
                for (pointer element_ptr = 0; element_ptr < new_element_ptr;
                     ++element_ptr) {
                    destroy(element_ptr);
                }

                deallocate(new_data, new_capacity);
                throw;
            }
        }

        clear();

        using std::swap;
        swap(data_, new_data);

        deallocate(new_data, new_capacity);

        capacity_ = new_capacity;
    }

    size_type capacity() const noexcept {
        return capacity_;
    }

    void shrink_to_fit() {
        if (full()) {
            return;
        }
    }

    void clear() noexcept {
        for (size_type index = 0; index < size(); ++index) {
            destroy(data_ + wrap_index(index));
        }

        size_ = 0;
        begin_ = 0;
    }

    void push_back(const value_type &value) {
        emplace_back(value);
    }

    void push_back(value_type &&value) {
        emplace_back(std::move(value));
    }

    template <typename ...Args,
              std::enable_if_t<std::is_constructible_v<value_type, Args...>,
                               int> = 0>
    void emplace_back(Args &&...args) {
        if (full()) {
            pop_front();
        }

        const pointer end = data_ + end_index();

        construct(end, std::forward<Args>(args)...);

        ++size_;
    }

    void push_front(const value_type &value) {
        emplace_front(value);
    }

    void push_front(value_type &&value) {
        emplace_front(std::move(value));
    }

    template <typename ...Args,
              std::enable_if_t<std::is_constructible_v<value_type, Args...>,
                               int> = 0>
    void emplace_front(Args &&...args) {
        if (full()) {
            pop_back();
        }

        const pointer reverse_end = data_ + rend_index();

        construct(reverse_end, std::forward<Args>(args)...);

        ++size_;
        --begin_;
        begin_ %= capacity_;
    }

    void pop_back() {
        if (empty()) {
            return;
        }

        TraitsT::destroy(allocator_, data_ + back_index());
        --size_;
    }

    void pop_front() {
        if (empty()) {
            return;
        }

        TraitsT::destroy(allocator_, data_ + front_index());
        --size_;
        ++begin_;
        begin_ %= capacity_;
    }

    // TODO: implement
    void resize(size_type count);

    // TODO: implement
    void resize(size_type count, const value_type &value);

    void swap(RingBuffer &other)
    noexcept(TraitsT::propagate_on_container_swap::value
             || TraitsT::is_always_equal::value) {
        using std::swap;

        swap(data_, other.data_);
        swap(size_, other.size_);
        swap(capacity_, other.capacity_);
        swap(begin_, other.begin_);

        if constexpr (TraitsT::propagate_on_container_swap::value
                      || !TraitsT::is_always_equal::value) {
            swap(allocator_, other.allocator_);
        }
    }

private:
    pointer allocate(const size_type count) {
        return TraitsT::allocate(allocator_, count);
    }

    void deallocate(const pointer ptr, const size_type count) {
        TraitsT::deallocate(allocator_, ptr, count);
    }

    template <typename U, typename ...Args>
    void construct(U *const ptr, Args &&...args) {
        TraitsT::construct(allocator_, ptr, std::forward<Args>(args)...);
    }

    template <typename U>
    void destroy(U *const ptr) {
        TraitsT::destroy(allocator_, ptr);
    }

    size_type front_index() const noexcept {
        return begin_;
    }

    size_type back_index() const noexcept {
        return wrap_index(size_ - 1);
    }

    size_type end_index() const noexcept {
        return wrap_index(size_);
    }

    size_type rend_index() const noexcept {
        return (begin_ - 1) % capacity_;
    }

    size_type wrap_index(const size_type index) const noexcept {
        return (begin_ + index) % capacity_;
    }

    void range_check(const size_type index) const {
        if (index >= size_) {
            throw std::out_of_range{ "RingBuffer::range_check" };
        }
    }

    bool full() const noexcept {
        return size_ == capacity_;
    }

    static pointer as_pointer(value_type *const raw_ptr) noexcept {
        return static_cast<pointer>(raw_ptr);
    }

    static const_pointer as_const_pointer(const value_type *const raw_ptr)
    noexcept {
        return static_cast<const_pointer>(raw_ptr);
    }

    pointer data_ = nullptr;
    size_type size_ = 0; // invariant: size <= capacity_
    size_type capacity_ = 0;
    size_type begin_ = 0; // invariant: begin_ < capacity_
    allocator_type allocator_{ };
};

template <typename T, typename Allocator>
void swap(RingBuffer<T, Allocator> &lhs, RingBuffer<T, Allocator> &rhs)
noexcept(noexcept(std::declval<RingBuffer<T, Allocator>&>()
             .swap(std::declval<RingBuffer<T, Allocator>&>()))) {
    lhs.swap(rhs);
}

} // namespace containers
} // namespace gregjm


#endif
