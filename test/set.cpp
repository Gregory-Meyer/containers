#include <catch.hpp>

#include "set.hpp"

#include <array>
#include <functional>
#include <string>
#include <string_view>

using namespace gregjm::containers;

TEST_CASE("Set insertion", "[Set]") {
    Set<int> integers;

    CHECK(integers.empty());
    CHECK(integers.size() == 0);

    CHECK(integers.insert(5).second);
    CHECK_FALSE(integers.empty());
    CHECK(integers.size() == 1);
    CHECK(integers.capacity() >= 1);
    CHECK(integers.count(5) == 1);
    CHECK(integers.count(10) == 0);

    CHECK(integers.insert(10).second);
    CHECK_FALSE(integers.empty());
    CHECK(integers.size() == 2);
    CHECK(integers.capacity() >= 2);
    CHECK(integers.count(5) == 1);
    CHECK(integers.count(10) == 1);
}

TEST_CASE("Set removal", "[Set]") {
    Set<int> integers{ 0, 1, 2, 3, 4 };

    CHECK_FALSE(integers.empty());
    CHECK(integers.size() == 5);
    CHECK(integers.capacity() >= 5);

    CHECK(integers.count(0) == 1);
    CHECK(integers.count(1) == 1);
    CHECK(integers.count(2) == 1);
    CHECK(integers.count(3) == 1);
    CHECK(integers.count(4) == 1);

    integers.erase(4);

    CHECK_FALSE(integers.empty());
    CHECK(integers.size() == 4);
    CHECK(integers.capacity() >= 5);
    CHECK(integers.count(0) == 1);
    CHECK(integers.count(1) == 1);
    CHECK(integers.count(3) == 1);
    CHECK(integers.count(2) == 1);
    CHECK(integers.count(4) == 0);

    integers.erase(4);

    CHECK_FALSE(integers.empty());
    CHECK(integers.size() == 4);
    CHECK(integers.capacity() >= 5);
    CHECK(integers.count(0) == 1);
    CHECK(integers.count(1) == 1);
    CHECK(integers.count(2) == 1);
    CHECK(integers.count(3) == 1);
    CHECK(integers.count(4) == 0);

    integers.erase(0);

    CHECK_FALSE(integers.empty());
    CHECK(integers.size() == 3);
    CHECK(integers.capacity() >= 5);
    CHECK(integers.count(0) == 0);
    CHECK(integers.count(1) == 1);
    CHECK(integers.count(2) == 1);
    CHECK(integers.count(3) == 1);
    CHECK_FALSE(integers.count(4) == 1);
}

TEST_CASE("Set resizing", "[Set]") {
    Set<int> integers;
    integers.reserve(16);

    CHECK(integers.capacity() >= 16);

    const std::array<int, 8> nums = { { 0, 1, 2, 3, 4, 5, 6, 7 } };

    integers.insert(nums.cbegin(), nums.cend());

    CHECK(integers.size() == 8);
    CHECK(integers.capacity() >= 16);

    for (const int num : nums) {
        CHECK(integers.count(num) == 1);
    }
}

TEST_CASE("heterogeneous lookup", "[Set]") {
    using namespace std::literals;

    const Set<std::string> strings{ "foo", "bar", "baz" };

    const std::hash<std::string_view> hasher;
    CHECK(strings.count("foo"sv, hasher) == 1);
    CHECK(strings.count("bar"sv, hasher) == 1);
    CHECK(strings.count("baz"sv, hasher) == 1);
    CHECK(strings.count("ayy"sv, hasher) == 0);

    CHECK(strings.find("foo"sv, hasher) != strings.cend());
    CHECK(*strings.find("foo"sv, hasher) == "foo");

    CHECK(strings.find("bar"sv, hasher) != strings.cend());
    CHECK(*strings.find("bar"sv, hasher) == "bar");

    CHECK(strings.find("baz"sv, hasher) != strings.cend());
    CHECK(*strings.find("baz"sv, hasher) == "baz");

    CHECK(strings.find("ayy"sv, hasher) == strings.cend());
}

TEST_CASE("Set iteration", "[Set]") {
    const Set<int> num_set{ 0, 1, 2, 3, 4, 5, 6, 7 };

    std::vector<int> num_vec(num_set.begin(), num_set.end());
    std::sort(num_vec.begin(), num_vec.end());

    const std::vector<int> num_vec_exp = { { 0, 1, 2, 3, 4, 5, 6, 7 } };

    CHECK(num_vec.size() == num_set.size());
    CHECK(std::equal(num_vec.cbegin(), num_vec.cend(), num_vec_exp.cbegin()));
}

TEST_CASE("heterogeneous erasure", "[Set]") {
    using namespace std::literals;

    Set<std::string> strings{ "foo", "bar", "baz" };

    const std::hash<std::string_view> hasher;

    strings.erase("foo"sv, hasher);

    CHECK(strings.count("foo") == 0);
    CHECK(strings.find("foo") == strings.cend());
}

TEST_CASE("clearing", "[Set]") {
    Set<int> numbers{ 0, 1, 2, 3 };

    CHECK(numbers.size() == 4);
    CHECK(numbers.capacity() >= 4);
    CHECK(numbers.find(0) != numbers.cend());
    CHECK(*numbers.find(0) == 0);

    CHECK(numbers.find(1) != numbers.cend());
    CHECK(*numbers.find(1) == 1);

    CHECK(numbers.find(2) != numbers.cend());
    CHECK(*numbers.find(2) == 2);

    CHECK(numbers.find(3) != numbers.cend());
    CHECK(*numbers.find(3) == 3);

    numbers.clear();

    CHECK(numbers.empty());
    CHECK(numbers.find(0) == numbers.cend());
    CHECK(numbers.find(1) == numbers.cend());
    CHECK(numbers.find(2) == numbers.cend());
    CHECK(numbers.find(3) == numbers.cend());
}

TEST_CASE("Set range insertion", "[Set]") {
    Set<int> numbers{ 0, 1, 2, 3 };
    const std::vector<int> to_insert{ 4, 5, 6, 7 };

    numbers.insert(to_insert.cbegin(), to_insert.cend());

    CHECK(numbers.size() == 8);
    CHECK(numbers.find(0) != numbers.cend());
    CHECK(numbers.find(1) != numbers.cend());
    CHECK(numbers.find(2) != numbers.cend());
    CHECK(numbers.find(3) != numbers.cend());
    CHECK(numbers.find(4) != numbers.cend());
    CHECK(numbers.find(5) != numbers.cend());
    CHECK(numbers.find(6) != numbers.cend());
    CHECK(numbers.find(7) != numbers.cend());
}
