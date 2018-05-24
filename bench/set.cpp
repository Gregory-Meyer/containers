#include "set.hpp"

#include <array>
#include <iterator>
#include <limits>
#include <memory>
#include <random>
#include <string>
#include <string_view>
#include <unordered_set>

#include <benchmark/benchmark.h>

template <typename ForwardIt, typename URBG>
static ForwardIt random_element(ForwardIt first, const ForwardIt last,
                                URBG &&g) {
    using DifferenceT =
        typename std::iterator_traits<ForwardIt>::difference_type;
    const auto size = std::distance(first, last);

    std::uniform_int_distribution<DifferenceT> dist{ 0, size };

    std::advance(first, dist(std::forward<URBG>(g)));

    return first;
}

template <typename Set, typename URBG>
static Set make_random_int_set(const std::size_t size, URBG &&g) {
    using ValueT = typename Set::value_type;

    static constexpr ValueT MIN = std::numeric_limits<ValueT>::min();
    static constexpr ValueT MAX = std::numeric_limits<ValueT>::max();

    std::uniform_int_distribution<ValueT> dist{ MIN, MAX };

    Set set;
    set.reserve(size);

    while (set.size() < size) {
        set.insert(dist(std::forward<URBG>(g)));
    }

    return set;
}

template <typename T, typename URBG>
T random_integer(URBG &&g, const T min = std::numeric_limits<T>::min(),
                 const T max = std::numeric_limits<T>::max()) {
    std::uniform_int_distribution<T> dist{ min, max };

    return dist(std::forward<URBG>(g));
}

template <typename T, typename URBG>
std::vector<T> random_integer_range(
    const std::size_t size, URBG &&g,
    const T min = std::numeric_limits<T>::min(),
    const T max = std::numeric_limits<T>::max()
) {
    std::uniform_int_distribution<T> dist{ min, max };

    std::vector<T> nums(size);
    std::generate(nums.begin(), nums.end(), [&dist, &g] { return dist(g); });

    return nums;
}

static void gregjm_int_set_find(benchmark::State &state) {
    using ValueT = int;
    using SetT = gregjm::containers::Set<ValueT>;

    auto engine_ptr = std::make_unique<std::mt19937_64>();

    for (auto _ : state) {
        state.PauseTiming();
        const auto size = static_cast<std::size_t>(state.range(0));
        const auto set = make_random_int_set<SetT>(size, *engine_ptr);
        const auto to_find = random_integer<ValueT>(*engine_ptr);
        state.ResumeTiming();

        benchmark::DoNotOptimize(set.find(to_find));
    }
}
BENCHMARK(gregjm_int_set_find)->Range(1, 1 << 12);

static void gregjm_int_set_insert(benchmark::State &state) {
    using ValueT = int;
    using SetT = gregjm::containers::Set<ValueT>;

    auto engine_ptr = std::make_unique<std::mt19937_64>();

    for (auto _ : state) {
        state.PauseTiming();
        const auto size = static_cast<std::size_t>(state.range(0));
        auto set = make_random_int_set<SetT>(size, *engine_ptr);
        const auto to_insert = random_integer<ValueT>(*engine_ptr);
        state.ResumeTiming();

        set.insert(to_insert);
    }
}
BENCHMARK(gregjm_int_set_insert)->Range(1, 1 << 12);

static void gregjm_int_set_insert_range(benchmark::State &state) {
    using ValueT = int;
    using SetT = gregjm::containers::Set<ValueT>;

    auto engine_ptr = std::make_unique<std::mt19937_64>();

    for (auto _ : state) {
        state.PauseTiming();
        const auto size = static_cast<std::size_t>(state.range(0));
        const auto range_size = static_cast<std::size_t>(state.range(1));
        auto set = make_random_int_set<SetT>(size, *engine_ptr);
        const auto to_insert = random_integer_range<ValueT>(range_size,
                                                            *engine_ptr);
        state.ResumeTiming();

        set.insert(to_insert.cbegin(), to_insert.cend());
    }
}
BENCHMARK(gregjm_int_set_insert_range)
    ->Ranges({ { 1, 1 << 12 }, { 1, 1 << 8 } });

static void std_int_set_find(benchmark::State &state) {
    using ValueT = int;
    using SetT = std::unordered_set<ValueT>;

    auto engine_ptr = std::make_unique<std::mt19937_64>();

    for (auto _ : state) {
        state.PauseTiming();
        const auto size = static_cast<std::size_t>(state.range(0));
        const auto set = make_random_int_set<SetT>(size, *engine_ptr);
        const auto to_find = random_integer<ValueT>(*engine_ptr);
        state.ResumeTiming();

        benchmark::DoNotOptimize(set.find(to_find));
    }
}
BENCHMARK(std_int_set_find)->Range(1, 1 << 12);

static void std_int_set_insert(benchmark::State &state) {
    using ValueT = int;
    using SetT = std::unordered_set<ValueT>;

    auto engine_ptr = std::make_unique<std::mt19937_64>();

    for (auto _ : state) {
        state.PauseTiming();
        const auto size = static_cast<std::size_t>(state.range(0));
        auto set = make_random_int_set<SetT>(size, *engine_ptr);
        const auto to_insert = random_integer<ValueT>(*engine_ptr);
        state.ResumeTiming();

        set.insert(to_insert);
    }
}
BENCHMARK(std_int_set_insert)->Range(1, 1 << 12);

static void std_int_set_insert_range(benchmark::State &state) {
    using ValueT = int;
    using SetT = std::unordered_set<ValueT>;

    auto engine_ptr = std::make_unique<std::mt19937_64>();

    for (auto _ : state) {
        state.PauseTiming();
        const auto size = static_cast<std::size_t>(state.range(0));
        const auto range_size = static_cast<std::size_t>(state.range(1));
        auto set = make_random_int_set<SetT>(size, *engine_ptr);
        const auto to_insert = random_integer_range<ValueT>(range_size,
                                                            *engine_ptr);
        state.ResumeTiming();

        set.insert(to_insert.cbegin(), to_insert.cend());
    }
}
BENCHMARK(std_int_set_insert_range)->Ranges({ { 1, 1 << 12 }, { 1, 1 << 8 } });

BENCHMARK_MAIN();
