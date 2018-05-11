#ifndef GREGJM_CONTAINERS_TRAITS_HPP
#define GREGJM_CONTAINERS_TRAITS_HPP

#include <iterator>
#include <type_traits>

namespace gregjm {
namespace containers {

template <typename T, typename = void>
struct IsContiguousContainer : public std::false_type { };

template <typename T>
struct IsContiguousContainer<T,
                             std::void_t<decltype(std::declval<T>().begin()),
                                         decltype(std::declval<T>().end()),
                                         decltype(std::declval<T>().data())>>
: public std::true_type { };

template <typename T>
constexpr inline bool IS_CONTIGUOUS_CONTAINER
    = IsContiguousContainer<T>::value;

template <typename T>
struct IsInputIterator
: public std::conditional_t<
    std::is_base_of_v<std::input_iterator_tag,
                      typename std::iterator_traits<T>::iterator_category>,
    std::true_type, std::false_type
> { };

template <typename T>
constexpr inline bool IS_INPUT_ITERATOR = IsInputIterator<T>::value;

} // namespace containers
} // namespace gregjm

#endif
