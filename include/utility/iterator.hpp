#ifndef GREGJM_CONTAINERS_UTILITY_ITERATOR_HPP
#define GREGJM_CONTAINERS_UTILITY_ITERATOR_HPP

#include <iterator>
#include <type_traits>

namespace gregjm::containers {
inline namespace utility {

template <typename T, typename = void>
struct IteratorDifferenceType { };

template <typename T>
struct IteratorDifferenceType<
    T, std::void_t<typename std::iterator_traits<T>::difference_type>
> {
    using type = typename std::iterator_traits<T>::difference_type;
};

template <typename T>
using IteratorDifferenceTypeT = typename IteratorDifferenceType<T>::type;

template <typename T, typename = void>
struct IteratorValueType { };

template <typename T>
struct IteratorValueType<
    T, std::void_t<typename std::iterator_traits<T>::value_type>
> {
    using type = typename std::iterator_traits<T>::value_type;
};

template <typename T>
using IteratorValueTypeT = typename IteratorValueType<T>::type;

template <typename T, typename = void>
struct IteratorPointer { };

template <typename T>
struct IteratorPointer<
    T, std::void_t<typename std::iterator_traits<T>::pointer>
> {
    using type = typename std::iterator_traits<T>::pointer;
};

template <typename T>
using IteratorPointerT = typename IteratorPointer<T>::type;

template <typename T, typename = void>
struct IteratorReference { };

template <typename T>
struct IteratorReference<
    T, std::void_t<typename std::iterator_traits<T>::reference>
> {
    using type = typename std::iterator_traits<T>::reference;
};

template <typename T>
using IteratorReferenceT = typename IteratorReference<T>::type;

template <typename T, typename = void>
struct IteratorCategory { };

template <typename T>
struct IteratorCategory<
    T, std::void_t<typename std::iterator_traits<T>::iterator_category>
> {
    using type = typename std::iterator_traits<T>::iterator_category;
};

template <typename T>
using IteratorCategoryT = typename IteratorCategory<T>::type;

template <typename T>
struct IsIterator
: public std::conditional_t<
    std::is_base_of_v<std::input_iterator_tag, IteratorCategoryT<T>>
    || std::is_base_of_v<std::output_iterator_tag, IteratorCategoryT<T>>,
    std::true_type, std::false_type
> { };

template <typename T>
constexpr inline bool IS_ITERATOR = IsIterator<T>::value;

template <typename T>
struct IsInputIterator
: public std::conditional_t<
    std::is_base_of_v<std::input_iterator_tag, IteratorCategoryT<T>>,
    std::true_type, std::false_type
> { };

template <typename T>
constexpr inline bool IS_INPUT_ITERATOR = IsInputIterator<T>::value;

template <typename T>
struct IsOutputIterator
: public std::conditional_t<
    std::is_base_of_v<std::output_iterator_tag, IteratorCategoryT<T>>,
    std::true_type, std::false_type
> { };

template <typename T>
constexpr inline bool IS_OUTPUT_ITERATOR = IsOutputIterator<T>::value;

template <typename T>
struct IsForwardIterator
: public std::conditional_t<
    std::is_base_of_v<std::forward_iterator_tag, IteratorCategoryT<T>>,
    std::true_type, std::false_type
> { };

template <typename T>
constexpr inline bool IS_FORWARD_ITERATOR = IsForwardIterator<T>::value;

template <typename T>
struct IsBidirectionalIterator
: public std::conditional_t<
    std::is_base_of_v<std::bidirectional_iterator_tag, IteratorCategoryT<T>>,
    std::true_type, std::false_type
> { };

template <typename T>
constexpr inline bool IS_BIDIRECTIONAL_ITERATOR
    = IsBidirectionalIterator<T>::value;

template <typename T>
struct IsRandomAccessIterator
: public std::conditional_t<
    std::is_base_of_v<std::random_access_iterator_tag, IteratorCategoryT<T>>,
    std::true_type, std::false_type
> { };

template <typename T>
constexpr inline bool IS_RANDOM_ACCESS_ITERATOR
    = IsRandomAccessIterator<T>::value;

} // inline namespace utility
} // namespace gregjm::containers

#endif
