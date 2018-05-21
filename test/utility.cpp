#include <catch.hpp>

#include "utility.hpp"

#include <algorithm>
#include <array>
#include <iterator>
#include <sstream>

using namespace gregjm::containers;

TEST_CASE("is_power_of_2", "[utility.hpp]") {
    CHECK(is_power_of_2(1ull));
    CHECK(is_power_of_2(2ull));
    CHECK(is_power_of_2(4ull));
    CHECK(is_power_of_2(8ull));

    CHECK(!is_power_of_2(0ull));
    CHECK(!is_power_of_2(15ull));
    CHECK(!is_power_of_2(17ull));
    CHECK(!is_power_of_2(7ull));
}

TEST_CASE("next_power_of_2", "[utility.hpp]") {
    CHECK(next_power_of_2(1ull) == 1ull);
    CHECK(next_power_of_2(2ull) == 2ull);
    CHECK(next_power_of_2(4ull) == 4ull);
    CHECK(next_power_of_2(8ull) == 8ull);

    CHECK(next_power_of_2(15ull) == 16ull);
    CHECK(next_power_of_2(17ull) == 32ull);
    CHECK(next_power_of_2(7ull) == 8ull);
    CHECK(next_power_of_2(9ull) == 16ull);
    CHECK(next_power_of_2(std::numeric_limits<unsigned long long>::max() - 1)
          == std::numeric_limits<unsigned long long>::max());
}

constexpr bool is_odd(const int x) noexcept {
    return x % 2 != 0;
}

TEST_CASE("Range usage with std::vector", "[Range][utility.hpp]") {
    std::vector<int> nums{ 0, 1, 2, 3 };

    const auto r1 = make_range(nums);
    const auto r2 = make_range(nums.begin(), nums.end());
    const auto r3 = make_range(nums.cbegin(), nums.cend());

    const auto f1 = make_filter(r1, [](const auto i) { return i % 2 == 0; });
    static constexpr int f1_exp[] = { 0, 2 };

    CHECK(f1.has_size());
    CHECK(f1.size() == 2);

    const auto f2 = make_filter(r2.begin(), r2.end(), &is_odd);
    static constexpr int f2_exp[] = { 1, 3 };

    CHECK(f2.has_size());
    CHECK(f2.size() == 2);

    const auto f3 = make_filter(std::begin(r3), std::end(r3),
                                [](auto) { return true; });
    static constexpr int f3_exp[] = { 0, 1, 2, 3};
    
    CHECK(f3.has_size());
    CHECK(f3.size() == 4);

    const std::vector<int> n1(f1.begin(), f1.end());
    const std::vector<int> n2(std::begin(f2), std::end(f2));
    const std::vector<int> n3(f3.begin(), f3.end());

    CHECK(std::equal(n1.begin(), n1.end(), std::begin(f1_exp)));
    CHECK(std::equal(n2.begin(), n2.end(), std::begin(f2_exp)));
    CHECK(std::equal(n3.begin(), n3.end(), std::begin(f3_exp)));
}

TEST_CASE("Range usage with istreams", "[Range][utility.hpp]") {
    std::istringstream iss{ "0 1 2 3" };

    const std::istream_iterator<int> first{ iss };
    const std::istream_iterator<int> last{ };

    const auto range = make_range(first, last);

    CHECK_FALSE(range.has_size());

    const auto even =
        make_filter(range, [](const auto x) { return x % 2 == 0; });
    const auto odd = make_filter(first, last, &is_odd);

    CHECK_FALSE(even.has_size());
    CHECK_FALSE(odd.has_size());

    static constexpr std::array<int, 2> even_exp = { { 0, 2 } };
    CHECK(std::equal(even.begin(), even.end(), even_exp.cbegin()));

    static constexpr std::array<int, 0> odd_exp;
    CHECK(std::equal(odd.begin(), odd.end(), odd_exp.cbegin()));
}
