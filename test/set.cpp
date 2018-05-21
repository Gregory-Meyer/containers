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

    CHECK(integers.insert(5));
    CHECK_FALSE(integers.empty());
    CHECK(integers.size() == 1);
    CHECK(integers.capacity() >= 2);
    CHECK(integers.contains(5));
    CHECK_FALSE(integers.contains(10));

    CHECK(integers.insert(10));
    CHECK_FALSE(integers.empty());
    CHECK(integers.size() == 2);
    CHECK(integers.capacity() >= 4);
    CHECK(integers.contains(5));
    CHECK(integers.contains(10));
}

TEST_CASE("Set removal", "[Set]") {
    Set<int> integers{ 0, 1, 2, 3, 4 };

    CHECK_FALSE(integers.empty());
    CHECK(integers.size() == 5);
    CHECK(integers.capacity() >= 10);

    CHECK(integers.contains(0));
    CHECK(integers.contains(1));
    CHECK(integers.contains(2));
    CHECK(integers.contains(3));
    CHECK(integers.contains(4));

    CHECK(integers.erase(4));
    CHECK_FALSE(integers.empty());
    CHECK(integers.size() == 4);
    CHECK(integers.capacity() >= 8);
    CHECK(integers.contains(0));
    CHECK(integers.contains(1));
    CHECK(integers.contains(2));
    CHECK(integers.contains(3));
    CHECK_FALSE(integers.contains(4));

    CHECK_FALSE(integers.erase(4));
    CHECK_FALSE(integers.empty());
    CHECK(integers.size() == 4);
    CHECK(integers.capacity() >= 8);
    CHECK(integers.contains(0));
    CHECK(integers.contains(1));
    CHECK(integers.contains(2));
    CHECK(integers.contains(3));
    CHECK_FALSE(integers.contains(4));

    CHECK(integers.erase(0));
    CHECK_FALSE(integers.empty());
    CHECK(integers.size() == 3);
    CHECK(integers.capacity() >= 6);
    CHECK_FALSE(integers.contains(0));
    CHECK(integers.contains(1));
    CHECK(integers.contains(2));
    CHECK(integers.contains(3));
    CHECK_FALSE(integers.contains(4));
}

TEST_CASE("Set resizing", "[Set]") {
    Set<int> integers;
    integers.reserve(16);

    CHECK(integers.capacity() >= 32);

    const std::array<int, 8> nums = { { 0, 1, 2, 3, 4, 5, 6, 7 } };

    integers.insert(nums.cbegin(), nums.cend());

    CHECK(integers.size() == 8);
    CHECK(integers.capacity() >= 32);

    for (const int num : nums) {
        CHECK(integers.contains(num));
    }
}

TEST_CASE("heterogeneous lookup", "[Set]") {
    using namespace std::literals;

    const Set<std::string> strings{ "foo", "bar", "baz" };

    const std::hash<std::string_view> hasher;
    CHECK(strings.contains("foo"sv, hasher));
    CHECK(strings.contains("bar"sv, hasher));
    CHECK(strings.contains("baz"sv, hasher));
    CHECK_FALSE(strings.contains("ayy"sv, hasher));
}
